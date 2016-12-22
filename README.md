# Automated-Bug-Detection-Tool

Inferring Likely Invariants for Bug Detection

void scope1() {
A(); B(); C(); D();
}

void scope2() {
A(); C(); D();
}

void scope3() {
A(); B(); B();
}

void scope4() {
B(); D(); scope1();
}

void scope5() {
B(); D(); A();
}

void scope6() {
B(); D();
}

We can learn that function A and function B are called together three times in function scope1, scope3, and scope5.
Function A is called four times in function scope1, scope2, scope3, and scope5. We infer that the one time that function
A is called without B in scope2 is a bug, as function A and B are called together 3 times. Please note that we only count
B once in scope3 although scope3 calls B two times.
