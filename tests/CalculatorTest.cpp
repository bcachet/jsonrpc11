#include <catch.hpp>

#include <Calculator.h>


SCENARIO("Calculator", "[calculator]") {
  GIVEN("Default Calculator") {
    Calculator calc;
    int result;
    calc.Clear();

    WHEN("Computing 2/2") {
      calc.Push(2); calc.Push(2);
      result = (int)calc.Divide();
      THEN("Result is 1") {
        REQUIRE(result == 1);
      }
    }

    WHEN("Computing 2+2") {
      calc.Push(2); calc.Push(2);
      result = (int)calc.Add();
      THEN("Result is 4") {
        REQUIRE(result == 4);
      }
    }
  }
}
