# symbolic
Symbolic calculator for arithmetic operations with fractions

## Updates
- Can accept symbolic variables with/without assigned values.
- Can display them in a fraction.

## Future Updates
- Can display current inputs.
- Can reduce fractions.
- Can apply values to variables for computation.
- Can display arithmetic operations ('+', '-', '\*', '/')
- Can scanf the "x = 1.2e-3" forms.
- Can parse line string inputs such as "G M m / r^2"
- Can parse equations of the form "F = G M m / r^2"
- Can reduce equations.

## Discussion
- Should I use hashtabs instead of binary trees for symbols?
	- Binary Tree
		- Pros
			- No hard-coded limit of variables as long as hardware supports.
			- Can sort variables easily when displaying.
		- Cons
			- When comparing two trees node-wise, it could be expensive.
				-> Define a look-up function of a given tree?
	- Hashtab
		- Pros
			- Comparing two trees node-wise is linear.
		- Cons
			- Pre-defined limited number of variables.
				-> Do we ever need tens of thousands of symbolic variables in practice?
- Should I give-up the fraction form and simplify the structure with accepting negative powers?
	- And fraction is just an option for printing?
	- Pros
		- One root tree to rule them all. (no num, denom, LHS, RHS, etc.)
	- Cons
		- increased member size for location info? (LHS, RHS?)
