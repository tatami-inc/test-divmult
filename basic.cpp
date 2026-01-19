#include "eztimer/eztimer.hpp"

#include "CLI/App.hpp"
#include "CLI/Formatter.hpp"
#include "CLI/Config.hpp"

#include <random>
#include <vector>
#include <optional>
#include <iostream>

int main(int argc, char ** argv) {
    CLI::App app{"Division/multiplication performance tests"};
    std::size_t len;
    app.add_option("-l,--length", len, "Length of the simulated vector")->default_val(10000000);
    int iterations;
    app.add_option("-i,--iter", iterations, "Number of iterations")->default_val(10);
    unsigned long long seed;
    app.add_option("-s,--seed", seed, "Seed for the simulated data")->default_val(69);
    CLI11_PARSE(app, argc, argv);

    // Setting up the simulated data.
    std::vector<double> inputs(len);
    std::mt19937_64 rng(seed);
    std::uniform_real_distribution<> unif;
    for (auto& i : inputs) {
        i = unif(rng);
    }

    const double denom = unif(rng);
    const double denom2 = 1.0/denom;

    // Setting up the functions.
    std::vector<std::function<double()> > funs;
    funs.reserve(2);

    std::vector<double> div_results(len);
    funs.emplace_back([&]() -> double {
        for (std::size_t i = 0; i < len; ++i) {
            div_results[i] = inputs[i] / denom;
        }
        return div_results.front() + div_results[len/2] + div_results.back();
    });

    std::vector<double> mult_results(len);
    funs.emplace_back([&]() -> double {
        for (std::size_t i = 0; i < len; ++i) {
            mult_results[i] = inputs[i] * denom2;
        }
        return mult_results.front() + mult_results[len/2] + mult_results.back();
    });

    // Performing the iterations.
    eztimer::Options opt;
    opt.iterations = iterations;

    std::vector<std::optional<double> > results(funs.size());
    auto res = eztimer::time<double>(
        funs,
        [&](const double& res, std::size_t i) -> void {
            if (results[i].has_value()) {
                if (*(results[i]) != res) {
                    throw std::runtime_error("oops that's not right");
                }
            } else {
                results[i] = res;
            }
        },
        opt
    );

    std::cout << "Division:      \t" << res[0].mean.count() << " ± " << res[0].sd.count() / std::sqrt(res[0].times.size()) << std::endl;
    std::cout << "Multiplication:\t" << res[1].mean.count() << " ± " << res[1].sd.count() / std::sqrt(res[1].times.size()) << std::endl;

    return 0;
}
