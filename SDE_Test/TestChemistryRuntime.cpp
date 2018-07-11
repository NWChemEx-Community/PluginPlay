#include <SDE/BasisSetFileParser.hpp>
#include <SDE/NWXDefaults.hpp> // Makes a non-defaulted ChemRuntime instance
#include <catch/catch.hpp>
#include <sstream>

using namespace SDE;

// H and O from cc-pvdz.gbs
std::string dz_string =
  "!  cc-pVDZ  EMSL  Basis Set Exchange Library   3/13/18 8:43 AM\n"
  "! H     : T.H. Dunning, Jr. J. Chem. Phys. 90, 1007 (1989).\n"
  "! Li - Ne: T.H. Dunning, Jr. J. Chem. Phys. 90, 1007 (1989).\n"
  "! \n"
  "****\n"
  "H     0 \n"
  "S   4   1.00\n"
  "     13.0100000              0.0196850        \n"
  "      1.9620000              0.1379770        \n"
  "      0.4446000              0.4781480        \n"
  "      0.1220000              0.5012400        \n"
  "S   1   1.00\n"
  "      0.1220000              1.0000000        \n"
  "P   1   1.00\n"
  "      0.7270000              1.0000000        \n"
  "****\n"
  "O     0 \n"
  "S   9   1.00\n"
  "  11720.0000000              0.0007100        \n"
  "   1759.0000000              0.0054700        \n"
  "    400.8000000              0.0278370        \n"
  "    113.7000000              0.1048000        \n"
  "     37.0300000              0.2830620        \n"
  "     13.2700000              0.4487190        \n"
  "      5.0250000              0.2709520        \n"
  "      1.0130000              0.0154580        \n"
  "      0.3023000             -0.0025850        \n"
  "S   9   1.00\n"
  "  11720.0000000             -0.0001600        \n"
  "   1759.0000000             -0.0012630        \n"
  "    400.8000000             -0.0062670        \n"
  "    113.7000000             -0.0257160        \n"
  "     37.0300000             -0.0709240        \n"
  "     13.2700000             -0.1654110        \n"
  "      5.0250000             -0.1169550        \n"
  "      1.0130000              0.5573680        \n"
  "      0.3023000              0.5727590        \n"
  "S   1   1.00\n"
  "      0.3023000              1.0000000        \n"
  "P   4   1.00\n"
  "     17.7000000              0.0430180        \n"
  "      3.8540000              0.2289130        \n"
  "      1.0460000              0.5087280        \n"
  "      0.2753000              0.4605310        \n"
  "P   1   1.00\n"
  "      0.2753000              1.0000000        \n"
  "D   1   1.00\n"
  "      1.1850000              1.0000000        \n"
  "****\n";

/*
 * CRT is basically just a giant collection of hard-coded data, testing of
 * which doesn't make any sense as the tests would be generated by the same
 * scripts used to hard-code the data.  Instead, this test focuses on making
 * sure the intended workflow works as intended.
 */
TEST_CASE("Workflow") {
    // Step 1 is always make a crt instance
    auto crt = default_runtime();

    // Step 2 is usually somehow get a molecule
    auto water = default_molecules().at("water");
    REQUIRE(water.atoms.size() == 3);

    // Step 3 is apply a basis set to that molecule
    auto water_w_basis = apply_basis(water, "cc-pvdz");

    // We just spot check a few things
    REQUIRE(water_w_basis.get_basis("cc-pvdz").size() == 24);

    // Alternatively can read basis from istream
    std::stringstream is(dz_string);
    water_w_basis = apply_basis_istream(water, "cc-pvdz-file", is, G94(), crt);
    REQUIRE(water_w_basis.get_basis("cc-pvdz-file").size() == 24);
}