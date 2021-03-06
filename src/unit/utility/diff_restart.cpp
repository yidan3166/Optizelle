#include <cstdlib>
#include <iostream>
#include <fstream>
#include <cmath>
#include <iomanip>
#include <sstream>
#include "optizelle/json.h"
#include "optizelle/vspaces.h"

int main(int argc,char* argv[]) {
    // Do some type shortcuts
    typedef size_t Natural;
    typedef Optizelle::Rm <double> Rm;

    // Make sure we have the correct number of arguments
    if(argc!=3) {
        std::cout << "diff_restart <baseline> <test>" << std::endl;
        return EXIT_FAILURE;
    }

    // Parse the JSON files 
    Json::Value baseline=Optizelle::json::parse(argv[1]);
    Json::Value test = Optizelle::json::parse(argv[2]);

    // Set a tolerance for our differences
    const double tol=0.05;

    // Loop over each of the elements in the baseline
    for(Json::ValueIterator category=baseline.begin();
        category!=baseline.end();
        category++
    ) {
        // Loop over each of the elements in the category we're comparing
        for(Json::ValueIterator variable=(*category).begin();
            variable!=(*category).end();
            variable++
        ) {
            if(category.key().asString()=="Naturals") {
                Natural x = (*variable).asUInt64();
                Natural y = test[category.name()][variable.name()]
                    .asUInt64();
                if(x!=y) {
                    std::cout << "Mismatch in " << category.name()
                        << '.' << variable.name() << ": "
                        << x << " != " << y << '.' << std::endl;
                    return EXIT_FAILURE;
                }
            } else if(category.key().asString()=="Reals") {
                double x = std::abs((*variable).asDouble());
                double y = test[category.name()][variable.name()]
                    .asDouble();

                if(std::abs(x-y) > tol * std::abs(x)) {
                    std::stringstream sin;
                    sin << tol;
                    std::cout << "Mismatch in " << category.name()
                        << std::scientific << std::setprecision(4) 
                        << '.' << variable.name()
                        << ": | " << x << " - " << y << " | > "
                        << sin.str() << " | " << x << " |." << std::endl;
                    return EXIT_FAILURE;
                }
            } else if(category.key().asString()=="Parameters") {
                std::string x = (*variable).asString();
                std::string y =
                    test[category.name()][variable.name()]
                    .asString();
                if(x!=y) {
                    std::cout << "Mismatch in " << category.name()
                        << '.' << variable.name() << ": "
                        << x << " != " << y << '.' << std::endl;
                    return EXIT_FAILURE;
                }
            } else if(category.key().asString()=="X_Vectors") {
                Json::StyledWriter writer;

                Rm::Vector x(
                    Optizelle::json::Serialization <double,Optizelle::Rm>
                        ::deserialize(
                            std::vector <double>(),
                            writer.write(baseline[category.name()]
                                                 [variable.name()])));
                Rm::Vector y(
                    Optizelle::json::Serialization <double,Optizelle::Rm>
                        ::deserialize(
                            std::vector <double>(),
                            writer.write(test[category.name()]
                                             [variable.name()])));
                // Check that the vectors are the same size.  Mostly, this
                // comes up when copying old setups for new tests.
                if(x.size()!=y.size()) {
                    std::cout << "Mismatch in " << category.name()
                        << '.' << variable.name()
                        << ": size(x)=" << x.size() << ", size(y)=" << y.size()
                        << std::endl;
                    return EXIT_FAILURE;
                }

                // Find the difference between the two vectors
                Rm::Vector diff(Rm::init(x));
                    Rm::copy(x,diff);
                    Rm::axpy(-1.,y,diff);
                if(Rm::innr(diff,diff) > tol * tol * Rm::innr(x,x)) {
                    std::cout << "Mismatch in " << category.name()
                        << '.' << variable.name()
                        << ": || base - test || > " 
                        << tol << " || base ||" << std::endl; 
                    return EXIT_FAILURE;
                }
            } 
        }
    }

    // Assuming we got through everything, exit successfully
    return EXIT_SUCCESS;
}
