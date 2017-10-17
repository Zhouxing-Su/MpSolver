#include "MpSolver.h"


using namespace std;


namespace szx {

GRBEnv MpSolver::globalEnv;


void MpSolver::updateStatus() {
    switch (model.get(GRB_IntAttr_Status)) {
    case GRB_OPTIMAL:
        status = Status::Optimal;
        return;
    case GRB_SUBOPTIMAL:
        status = Status::Feasible;
        return;
    case GRB_LOADED:
    case GRB_INPROGRESS:
        status = Status::Proceeding;
        break;
    case GRB_ITERATION_LIMIT:
    case GRB_NODE_LIMIT:
    case GRB_TIME_LIMIT:
    case GRB_SOLUTION_LIMIT:
        status = Status::ExceedLimit;
        break;
    case GRB_CUTOFF:
        status = Status::InsolubleCutoff;
        break;
    case GRB_INFEASIBLE:
    case GRB_INF_OR_UNBD:
    case GRB_UNBOUNDED:
        status = Status::InsolubleModel;
        break;
    default:
        status = Status::Error;
        break;
    }
    if (model.get(GRB_IntAttr_SolCount) > 0) { status = Status::Feasible; }
}

MpSolver::DecisionVar MpSolver::addDecisionVar(const DecisionVarInfo &decisionVarInfo) {
    return model.addVar(decisionVarInfo.lowerBound, decisionVarInfo.upperBound,
        decisionVarInfo.objectiveCoefficient, decisionVarInfo.numberType);
}

MpSolver::DecisionVar MpSolver::addDecisionVar(double lowerBound, double upperBound, double objectiveCoefficient, NumberType numberType) {
    return model.addVar(lowerBound, upperBound, objectiveCoefficient, numberType);
}

MpSolver::DecisionVar MpSolver::addDecisionVar(double lowerBound, double upperBound, double objectiveCoefficient, NumberType numberType, const string &name) {
    return model.addVar(lowerBound, upperBound, objectiveCoefficient, numberType, name);
}

MpSolver::DecisionVar MpSolver::addDecisionVar(double lowerBound, double upperBound, double objectiveCoefficient, NumberType numberType,
    const Constraint &constraint, double coefficient) {
    return model.addVar(lowerBound, upperBound, objectiveCoefficient, numberType, 1, &constraint, &coefficient);
}

MpSolver::DecisionVar MpSolver::addDecisionVar(double lowerBound, double upperBound, double objectiveCoefficient, NumberType numberType,
    const vector<Constraint> &constraints) {
    return model.addVar(lowerBound, upperBound, objectiveCoefficient, numberType, static_cast<int>(constraints.size()), constraints.data(), vector<double>(constraints.size(), 1).data());
}

MpSolver::DecisionVar MpSolver::addDecisionVar(double lowerBound, double upperBound, double objectiveCoefficient, NumberType numberType,
    const vector<Constraint> &constraints, const vector<double> &coefficients) {
    return model.addVar(lowerBound, upperBound, objectiveCoefficient, numberType, static_cast<int>(constraints.size()), constraints.data(), coefficients.data());
}

Arr<MpSolver::DecisionVar> MpSolver::addDecisionVars(const vector<DecisionVarInfo>& decisionVarInfos) {
    Arr<DecisionVar> vars(static_cast<int>(decisionVarInfos.size()));
    for (int i = 0; i < vars.size(); ++i) {
        vars.at(i) = addDecisionVar(decisionVarInfos.at(i));
    }
    return vars;
}

Arr<MpSolver::DecisionVar> MpSolver::addDecisionVars(int count, const double *lowerBounds, const double *upperBounds,
    const double *objectiveCoefficients, const NumberType *numberType) {
    vector<char> numType(numberType, numberType + count);
    return Arr<DecisionVar>(count, model.addVars(lowerBounds, upperBounds, objectiveCoefficients, numType.data(), nullptr, count));
}

MpSolver::Constraint MpSolver::addConstraint(const LogicalExpr &expr) {
    return model.addConstr(expr);
}

MpSolver::Constraint MpSolver::addConstraint(const LinearExpr &expr, double lowerBound, double upperBound) {
    return model.addRange(expr, lowerBound, upperBound);
}

MpSolver::Constraint MpSolver::addConstraint(const LinearExpr &expr,
    const LinearExpr &lowerBound, const LinearExpr &upperBound) {
    model.addConstr(lowerBound <= expr);
    model.addConstr(expr <= upperBound);

    throw GRBException("Adding range constraint with expr bounds is not implemented by Gurobi."
        "\nUse 2 standard constraints instead.");
}

}
