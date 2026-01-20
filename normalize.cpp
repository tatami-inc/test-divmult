#include "eztimer/eztimer.hpp"

#include "CLI/App.hpp"
#include "CLI/Formatter.hpp"
#include "CLI/Config.hpp"

#include <random>
#include <vector>
#include <optional>
#include <iostream>

int main(int argc, char ** argv) {
    CLI::App app{"Log-normalization performance tests"};
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
    constexpr double ldenom = std::log(2);
    constexpr double ldenom2 = 1 / ldenom;

    // Setting up the functions.
    std::vector<std::function<double()> > funs;
    funs.reserve(2);

    std::vector<double> log1pdiv_results(len);
    funs.emplace_back([&]() -> double {
        for (std::size_t i = 0; i < len; ++i) {
            log1pdiv_results[i] = std::log1p(inputs[i] / denom) / ldenom;
        }
        return log1pdiv_results.front() + log1pdiv_results[len/2] + log1pdiv_results.back();
    });

    std::vector<double> log1pmult_results(len);
    funs.emplace_back([&]() -> double {
        for (std::size_t i = 0; i < len; ++i) {
            log1pmult_results[i] = std::log1p(inputs[i] * denom2) * ldenom2;
        }
        return log1pmult_results.front() + log1pmult_results[len/2] + log1pmult_results.back();
    });

    std::vector<double> log2div_results(len);
    funs.emplace_back([&]() -> double {
        for (std::size_t i = 0; i < len; ++i) {
            log2div_results[i] = std::log2(inputs[i] / denom + 1); 
        }
        return log2div_results.front() + log2div_results[len/2] + log2div_results.back();
    });

    std::vector<double> log2mult_results(len);
    funs.emplace_back([&]() -> double {
        for (std::size_t i = 0; i < len; ++i) {
            log2mult_results[i] = std::log2(inputs[i] * denom2 + 1); 
        }
        return log2mult_results.front() + log2mult_results[len/2] + log2mult_results.back();
    });

    std::vector<double> logdiv_results(len);
    funs.emplace_back([&]() -> double {
        for (std::size_t i = 0; i < len; ++i) {
            logdiv_results[i] = std::log(inputs[i] / denom + 1) / ldenom; 
        }
        return logdiv_results.front() + logdiv_results[len/2] + logdiv_results.back();
    });

    std::vector<double> logmult_results(len);
    funs.emplace_back([&]() -> double {
        for (std::size_t i = 0; i < len; ++i) {
            logmult_results[i] = std::log(inputs[i] * denom2 + 1) * ldenom2; 
        }
        return logmult_results.front() + logmult_results[len/2] + logmult_results.back();
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
                std::cout << i << " : " << res << std::endl;
            }
        },
        opt
    );

    std::cout << "log1p(x / f) / M:   " << res[0].mean.count() << " ± " << res[0].sd.count() / std::sqrt(res[0].times.size()) << std::endl;
    std::cout << "log1p(x * r) * N:   " << res[1].mean.count() << " ± " << res[1].sd.count() / std::sqrt(res[1].times.size()) << std::endl;
    std::cout << "log2(x / f + 1):    " << res[2].mean.count() << " ± " << res[2].sd.count() / std::sqrt(res[2].times.size()) << std::endl;
    std::cout << "log2(x * r + 1):    " << res[3].mean.count() << " ± " << res[3].sd.count() / std::sqrt(res[3].times.size()) << std::endl;
    std::cout << "log(x / f + 1) / M: " << res[4].mean.count() << " ± " << res[4].sd.count() / std::sqrt(res[4].times.size()) << std::endl;
    std::cout << "log(x * r + 1) * N: " << res[5].mean.count() << " ± " << res[5].sd.count() / std::sqrt(res[5].times.size()) << std::endl;

    return 0;
}

