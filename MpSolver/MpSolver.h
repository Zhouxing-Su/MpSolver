////////////////////////////////
/// usage : 1.	wrap Gurobi optimizer to be more self documenting and easier to use.
///         2.  configure Gurobi as http://www.gurobi.com/support/faqs instruct
///             (How do I configure a new Gurobi C++ project with Microsoft Visual Studio),
///             then just add MpSolver.h and MpSolver.cpp to your project to use.
///
/// note  : 1.	this project does not provide Gurobi lisence.
////////////////////////////////

#ifndef SZX_OPTIMIZER_MP_SOLVER_H
#define SZX_OPTIMIZER_MP_SOLVER_H


#include <vector>
#include <string>

#include "Utility.h"

#pragma region Flag
#define GUROBI_ROOT  ../Lib/gurobi/

/// only use the major and minor version number, e.g. v6.5.0 => 65, v6.0.5 => 60.
#define GUROBI_VERSION  65
#define GUROBI_LATEST_VERSION  65

#define VISUAL_STUDIO_VERSION  2015
#pragma endregion Flag


#pragma region AutoLinking
#if GUROBI_VERSION < GUROBI_LATEST_VERSION
#define GUROBI_DIR  RESOLVED_CONCAT3(GUROBI_ROOT, Old/, GUROBI_VERSION, /)
#else
#define GUROBI_DIR  GUROBI_ROOT
#endif // GUROBI_VERSION

#if _LL_DYNAMIC
#define LINK_TYPE  d
#else
#define LINK_TYPE  t
#endif // _LL_DYNAMIC

#if _DR_DEBUG
#define RUNTIME_LIB  RESOLVED_CONCAT(LINK_TYPE, d)
#else
#define RUNTIME_LIB  LINK_TYPE
#endif // NDEBUG


#include RESOLVED_STRINGIFY(RESOLVED_CONCAT(GUROBI_DIR, gurobi_c++.h))

#pragma comment(lib, RESOLVED_STRINGIFY(RESOLVED_CONCAT2(GUROBI_DIR, gurobi, GUROBI_VERSION)))
#pragma comment(lib, RESOLVED_STRINGIFY(RESOLVED_CONCAT3(GUROBI_DIR, gurobi_c++m, RUNTIME_LIB, VISUAL_STUDIO_VERSION)))
#pragma endregion AutoLinking


namespace szx {

class MpSolver {
    #pragma region Constant
public:
    /// Gurobi variable can be GRB_CONTINUOUS, GRB_BINARY, GRB_INTEGER, GRB_SEMICONT, or GRB_SEMIINT.
    /// Semi-continuous variables can take any value between the specified lower and upper bounds, or a value of zero. 
    /// Semi-integer variables can take any integer value between the specified lower and upper bounds, or a value of zero
    enum NumberType { Bool = GRB_BINARY, Integer = GRB_INTEGER, Real = GRB_CONTINUOUS };

    enum OptimaOrientation { Minimize = GRB_MINIMIZE, Maximize = GRB_MAXIMIZE };

    // EXTEND[szx][1]: use bitset instead of a single value to record the state.
    //                 since Feasible may conflict with ExceedLimit.
    /// status for the most recent optimization.
    enum Status {
        Optimal,         /// GRB_OPTIMAL
        Feasible,        /// GRB_SUBOPTIMAL || (SolutionCount > 0)
        Proceeding,      /// GRB_LOADED || GRB_INPROGRESS
        Ready,           /// ready for solve()
        ExceedLimit,     /// GRB_ITERATION_LIMIT || GRB_NODE_LIMIT || GRB_TIME_LIMIT || GRB_SOLUTION_LIMIT
        InsolubleCutoff, /// GRB_CUTOFF
        InsolubleModel,  /// GRB_INFEASIBLE || GRB_INF_OR_UNBD || GRB_UNBOUNDED
        OutOfMemory,     /// OUT_OF_MEMORY
        Error            /// any other status code or error code (you may not get it as exception is thrown)
    };

    /// objective of constraint violation minimization.
    enum RelaxObjType {
        Linear = GRB_FEASRELAX_LINEAR,          /// minimize the sum of the weighted magnitudes of the bound and constraint violations.
        Quadratic = GRB_FEASRELAX_QUADRATIC,    /// minimize the weighted sum of the squares of the bound and constraint violations.
        Cardinality = GRB_FEASRELAX_CARDINALITY /// minimize the weighted count of bound and constraint violations.
    };

    /// numeric limits.
    static constexpr int MaxInteger = GRB_MAXINT;
    static constexpr double MaxReal = GRB_INFINITY;

    /// default thread number to let Gurobi make the decision.
    static constexpr int AutoThreading = 0;

    #pragma endregion Constant

    #pragma region Type
public:
    // OPTIMIZE[szx][0]: check size of Gurobi types. pass by reference if they are big.
    using DecisionVar = GRBVar;

    struct DecisionVarInfo {
        double lowerBound;
        double upperBound;
        double objectiveCoefficient; // if the objective will be set by setObjective(), just leave it to 0.
        NumberType numberType;
    };

    using LinearExpr = GRBLinExpr;
    using LogicalExpr = GRBTempConstr;

    using Constraint = GRBConstr;
    #pragma endregion Type

    #pragma region Constructor
public:
    /// create an instance of model with the same default parameters as Gurobi,
    /// except output is turned off.
    MpSolver() : model(globalEnv), status(Status::Ready) {
        setOutput(false);
    }

    void loadModel(const std::string &filePath) { model.read(filePath); }
    void saveModel(const std::string &filePath) {
        updateModel();
        model.write(filePath);
    }
    #pragma endregion Constructor

    #pragma region Method
public:
    static bool solutionFound(Status status) {
        return ((status == MpSolver::Status::Optimal) || (status == MpSolver::Status::Feasible));
    }

    /// apply pending change to the model and optimize it.
    /// return true if there are any solutions found.
    bool solve() {
        try {
            model.optimize();
        } catch (const GRBException& e) {
            if (e.getErrorCode() == GRB_ERROR_OUT_OF_MEMORY) {
                status = Status::OutOfMemory;
            } else {
                status = Status::Error;
                throw e;
            }
        }
        updateStatus();
        return solutionFound(status);
    }

    /// if (optimizeOriginalObj == false), gives a solution that minimizes the cost of the violation.
    /// otherwise, finds a solution that minimizes the original objective only among those that minimize the violation.
    /// return 0 if (optimizeOriginalObj == false). otherwise, the return value is the objective value for
    /// the relaxation performed. return a value less than 0, if the method failed to create the feasibility relaxation.
    double relax(bool relaxVarBound = false, bool relaxConstraint = true,
        bool optimizeOriginalObj = false, RelaxObjType relaxobjtype = RelaxObjType::Linear) {
        model.update();
        return model.feasRelax(relaxobjtype, optimizeOriginalObj, relaxVarBound, relaxConstraint);
    }
    double relax(const std::vector<Constraint> &constr, const std::vector<double> &rhspen,
        const std::vector<DecisionVar> &vars, const std::vector<double> &lbpen, const std::vector<double> &ubpen,
        bool optimizeOriginalObj = false, RelaxObjType relaxobjtype = RelaxObjType::Linear) {
        model.update();
        return model.feasRelax(relaxobjtype, optimizeOriginalObj,
            static_cast<int>(vars.size()), vars.data(), lbpen.data(), ubpen.data(),
            static_cast<int>(constr.size()), constr.data(), rhspen.data());
    }
    double relax(const std::vector<Constraint> &constr, const std::vector<double> &rhspen,
        bool optimizeOriginalObj = false, RelaxObjType relaxobjtype = RelaxObjType::Linear) {
        model.update();
        return model.feasRelax(relaxobjtype, optimizeOriginalObj, 0, nullptr, nullptr, nullptr,
            static_cast<int>(constr.size()), constr.data(), rhspen.data());
    }
    double relax(const std::vector<DecisionVar> &vars, const std::vector<double> &lbpen, const std::vector<double> &ubpen,
        bool optimizeOriginalObj = false, RelaxObjType relaxobjtype = RelaxObjType::Linear) {
        model.update();
        return model.feasRelax(relaxobjtype, optimizeOriginalObj,
            static_cast<int>(vars.size()), vars.data(), lbpen.data(), ubpen.data(), 0, nullptr, nullptr);
    }

    /// apply pending modification to the model (automatically called in solve()).
    /// only call it when you are adding items referencing former items.
    void updateModel() { model.update(); }

    /// return status for the most recent optimization.
    Status getStatus() {
        if (status == Status::Ready) { updateStatus(); }
        return status;
    }

    #pragma region Objective
    double getObjectiveValue() const { return model.get(GRB_DoubleAttr_ObjVal); }

    /// override entire existing objective function (previous call or objectiveCoefficients in addDecisionVar() calls).
    void setObjective(const LinearExpr &expr) { model.setObjective(expr); }
    /// override entire existing objective function and optimaOrientation in setOptimaOrientation() call.
    void setObjective(const LinearExpr &expr, OptimaOrientation optimaOrientation) {
        model.setObjective(expr, optimaOrientation);
    }
    /// choose between minimization and maximization for the optimization.
    void setOptimaOrientation(OptimaOrientation optimaOrientation = OptimaOrientation::Minimize) {
        model.set(GRB_IntAttr_ModelSense, optimaOrientation);
    }
    /// set worst objective value of solutions to be record.
    /// set to MaxReal for unbounded minimization and -MaxReal for unbounded maximization.
    void setCutoff(double objBound) { model.getEnv().set(GRB_DoubleParam_Cutoff, objBound); }
    #pragma endregion Objective

    #pragma region Decision variable
    DecisionVar addDecisionVar(const DecisionVarInfo &decisionVarInfo);
    DecisionVar addDecisionVar(double lowerBound, double upperBound, double objectiveCoefficient, NumberType numberType);
    DecisionVar addDecisionVar(double lowerBound, double upperBound, double objectiveCoefficient, NumberType numberType, const std::string &name);
    DecisionVar addDecisionVar(double lowerBound, double upperBound, double objectiveCoefficient, NumberType numberType,
        const Constraint &constraint, double coefficient = 1.0);
    DecisionVar addDecisionVar(double lowerBound, double upperBound, double objectiveCoefficient, NumberType numberType,
        const std::vector<Constraint> &constraints);
    DecisionVar addDecisionVar(double lowerBound, double upperBound, double objectiveCoefficient, NumberType numberType,
        const std::vector<Constraint> &constraints, const std::vector<double> &coefficients);

    Arr<DecisionVar> addDecisionVars(const std::vector<DecisionVarInfo> &decisionVarInfos);
    Arr<DecisionVar> addDecisionVars(int count, const double *lowerBounds, const double *upperBounds,
        const double *objectiveCoefficients, const NumberType *numberType);

    /// return the number of feasible solution found so far.
    int getSolutionCount() const { return model.get(GRB_IntAttr_SolCount); }

    /// determines which alternate solution is retrieved when calling getAltValue().
    /// solutionIndex should be less than getSolutionCount().
    void setSolutionIndex(int solutionIndex) { model.getEnv().set(GRB_IntParam_SolutionNumber, solutionIndex); }

    /// get the value of decision variable in the best solution.
    static double getValue(const DecisionVar &dvar) { return dvar.get(GRB_DoubleAttr_X); }
    /// get the value of decision variable in the n_th best solution.
    static double getAltValue(const DecisionVar &dvar) { return dvar.get(GRB_DoubleAttr_Xn); }
    double getAltValue(const DecisionVar &dvar, int solutionIndex) {
        setSolutionIndex(solutionIndex);
        return getAltValue(dvar);
    }

    static bool isTrue(const DecisionVar &boolTypeDvar) { return (boolTypeDvar.get(GRB_DoubleAttr_X) > 0.5); }
    #pragma endregion Decision variable

    #pragma region Constraint
    Constraint addConstraint(const LogicalExpr &expr);
    Constraint addConstraint(const LinearExpr &expr, double lowerBound, double upperBound);
    Constraint addConstraint(const LinearExpr &expr, const LinearExpr &lowerBound, const LinearExpr &upperBound);

    int getConstraintNumber() const { return model.get(GRB_IntAttr_NumConstrs); }
    #pragma endregion Constraint

    #pragma region Limits
    /// set timeout (wall clock) for each solve() call.
    void setTimeout(double seconds = MaxReal) { model.getEnv().set(GRB_DoubleParam_TimeLimit, seconds); }

    /// Limits the number of feasible MIP solutions found.
    void setMaxSolutionCount(int solutionCount = MaxInteger) { model.getEnv().set(GRB_IntParam_SolutionLimit, solutionCount); }

    /// set threads available for the solver.
    void setMaxThread(int threadNum = AutoThreading) { model.getEnv().set(GRB_IntParam_Threads, threadNum); }
    #pragma endregion Limits

    /// set random seed for the solver.
    void setSeed(int seed) { model.getEnv().set(GRB_IntParam_Seed, seed); }

    /// (enable == true) to turn on output, run in silence mode otherwise.
    void setOutput(bool enable = true) { model.getEnv().set(GRB_IntParam_OutputFlag, enable); }

    // EXTEND[szx][0]: wrap methods to get value of decision variable.

protected:
    void updateStatus();
    #pragma endregion Method

    #pragma region Field
protected:
    static GRBEnv globalEnv;
    // EXTEND[szx][5]: make it thread_local?
    // EXTEND[szx][5]: add static mutex gurobiMutex; here for multithread support.
    // EXTEND[szx][5]: add lock_guard<mutex> gurobiGuard(gurobiMutex); to every function.

    /// definition of the problem to solve.
    GRBModel model;

    /// status for the most recent optimization.
    Status status;
    #pragma endregion Field
}; // MpSolver


/// reference: section "3.4. CHANGE OF VARIABLE METHOD" (page 78)
///            in "Fractional Programming Theory, Methods and Applications".
class LinearFractionalSolver : public MpSolver {
public:
    LinearFractionalSolver() : t(model.addVar(0, MaxReal, 0, NumberType::Real)) {}

    /// the objective is objNumerator/objDenominator.
    /// make sure constraints are the left hand side of A x - b <= 0.
    bool solve(LinearExpr &objNumerator, LinearExpr &objDenominator, std::vector<LinearExpr> &constraints,
        OptimaOrientation optimaOrientation = OptimaOrientation::Minimize, double gamma = 1.0) {
        // A y - b t <= 0.
        for (auto c = constraints.begin(); c != constraints.end(); ++c) {
            resetConstant(*c);
            model.addConstr(*c <= 0);
        }
        // d y + d0 t == \gamma.
        resetConstant(objDenominator);
        model.addConstr(objDenominator == gamma);
        // c y + c0 t.
        resetConstant(objNumerator);
        model.setObjective(objNumerator, optimaOrientation);
    }

    /// get the value of the original decision variable in the best solution.
    double getValue(const DecisionVar &dvar) { return getValue(dvar) / getValue(t); }

protected:
    /// (c x + c0) => (c x + c0 t).
    void resetConstant(LinearExpr &expr) {
        double c = expr.getConstant();
        expr += (c * t);
        expr -= c;
    }

    DecisionVar t;
};

}


#endif // SZX_OPTIMIZER_MP_SOLVER_H
