
/** Represents a JavaScript comparison operator */
export enum CompareOp
{
    /** Strict equality */
    Equal,
    /** [description] */
    NotEqual,
    GreaterThan,
    GreaterThanOrEqual,
    LessThan,
    LessThanOrEqual
}

/** Represents a JavaScript binary logical operator */
export enum LogicalOp
{
    Or,
    And,
}


/**
 * Conditional Statement that contains a binary conditional operator that
 * is to be evaluated.
 */
export interface Conditional
{
    /** Callback to retrieve left-side perand */
    left: () => any;

    /** operator */
    op: CompareOp;

    /** Callback to retrieve right-side operand */
    right: () => any;

    /**
     * Logical operator for comparison with next statement. If it is not
     * defined, the program will stop evaluating any further conditionals that
     * come after it.
     */
    logicalOp?: LogicalOp;
}

/**
 * Group of conditional statements
 */
export interface ConditionalGroup
{
    /** The group of statements */
    statements: (Conditional | ConditionalGroup)[];

    /**
     * Logical operator for comparison with next statement. If it is not
     * defined, the program will stop evaluating any further conditionals that
     * come after it.
     */
    logicalOp?: LogicalOp;
}

/** Use to distinguish between Statement and StatementGroup */
function isStatementGroup(obj: any): obj is ConditionalGroup
{
    return obj.statements !== undefined;
}

function evalLogicalOp(left: boolean, right: boolean, op: LogicalOp)
{
    switch(op)
    {
        case LogicalOp.And:
            return left && right;
        case LogicalOp.Or:
            return left || right;
        default:
            throw Error("Unknown joint operation");
    }
}

function evalComparison(left: any, right: any, op: CompareOp): boolean
{
    switch(op)
    {
        case CompareOp.Equal:
            return left === right;
        case CompareOp.GreaterThan:
            return left > right;
        case CompareOp.GreaterThanOrEqual:
            return left >= right;
        case CompareOp.LessThan:
            return left < right;
        case CompareOp.LessThanOrEqual:
            return left <= right;
        case CompareOp.NotEqual:
            return left !== right;
        default:
            throw Error("Unknown comparison operation");
    }
}

export function evalStatement(s: Conditional): boolean
{
    return evalComparison(s.left(), s.right(), s.op);
}

export function evalStatementGroup(group: ConditionalGroup): boolean
{
    if (group.statements.length === 0)
    {
        throw Error("StatementGroup is empty");
    }

    let lastEval: boolean | null = null;
    let lastLogicOp: LogicalOp | null = null;

    for (let i = 0; i < group.statements.length; ++i)
    {
        const s = group.statements[i];
        let currentEval: boolean;
        if (isStatementGroup(s))
        {
            currentEval = evalStatementGroup(s);
        }
        else
        {
            currentEval = evalStatement(s);
        }

        if (lastLogicOp !== null && lastEval !== null)
        {
            currentEval = evalLogicalOp(lastEval, currentEval, lastLogicOp);
        }

        // No connecting logical operator, return the result
        if (i > 0 && lastLogicOp === null)
        {
            return currentEval;
        }

        lastEval = currentEval;
        lastLogicOp = s.logicalOp ?? null;
    }

    return lastEval;
}
