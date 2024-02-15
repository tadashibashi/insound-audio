
/**
 * Represents a JavaScript comparison operator
 * Omits non-strict equality for better user experience
 */
export enum CompareOp
{
    /** Strict equality: === */
    Equal,
    /** Strict inequality: !== */
    NotEqual,
    /** > */
    GreaterThan,
    /** >= */
    GreaterThanOrEqual,
    /** < */
    LessThan,
    /** <= */
    LessThanOrEqual
}

/** Represents a JavaScript binary logical operator */
export enum LogicalOp
{
    Or,
    And,
}

/**
 * Contract for conditional statement, flexible to implement your own, or use
 * provided `Conditional` or `ConditionalGroup` classes.
 */
export interface IConditional
{
    /** Evaluate the result of the conditional */
    evaluate(): boolean;

    /**
     * Logical operator for comparison with next statement. If it is not
     * defined, the program will stop evaluating any further conditionals that
     * come after it.
     */
    logicalOp?: LogicalOp;

    /** Whether to implement logical `not` op on `evaluate` */
    not: boolean;
}

/**
 * Conditional Statement that contains a binary conditional operator that
 * is to be evaluated.
 */
export class Conditional implements IConditional
{
    constructor(left: () => any, right: () => any, op: CompareOp, not: boolean = false)
    {
        this.left = left;
        this.right = right;
        this.op = op;
        this.not = not;
    }

    /** Callback to retrieve left-side perand */
    left: () => any;

    /** operator */
    op: CompareOp;

    /** Callback to retrieve right-side operand */
    right: () => any;


    logicalOp?: LogicalOp;

    not: boolean;

    evaluate(): boolean
    {
        const result = evalComparison(this.left(), this.right(), this.op);
        return this.not ? !result : result;
    }
}

/**
 * Group of conditional statements
 */
export class ConditionalGroup implements IConditional
{
    constructor(statements?: IConditional[], logicalOp?: LogicalOp, not: boolean = false)
    {
        this.m_statements = statements || [];
        this.logicalOp = logicalOp;
        this.not = not;
    }

    /** The group of statements */
    private m_statements: IConditional[];

    get statements() { return this.m_statements; }

    erase(index: number)
    {
        this.m_statements.splice(index, 1);

        // Update statements for frontend reaction
        this.m_statements = this.m_statements;
    }

    push(conditional: IConditional)
    {
        this.m_statements.push(conditional);
        this.m_statements = this.m_statements;
    }

    clear()
    {
        this.m_statements.length = 0;
        this.m_statements = this.m_statements;
    }

    /**
     * Logical operator for comparison with next statement. If it is not
     * defined, the program will stop evaluating any further conditionals that
     * come after it.
     */
    logicalOp?: LogicalOp;

    not: boolean;

    evaluate(): boolean
    {
            if (this.m_statements.length === 0)
            {
                throw Error("Cannot evaluate an empty StatementGroup");
            }

        let lastEval: boolean | null = null;
        let lastLogicOp: LogicalOp | null = null;

        for (let i = 0; i < this.m_statements.length; ++i)
        {
            const s = this.m_statements[i];
            let currentEval = s.evaluate();

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

        return this.not ? !lastEval : lastEval;
    }
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
