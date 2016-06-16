#pragma once

#include <string>
#include <vector>
#include <memory>
#include "isxCoreFwd.h"

namespace isx {

    ///
    /// A class encapsulating an nVista movie file.
    ///
    class Algorithm
    {
    public:
        /// Default constructor.  Is a valid C++ object but not usable.
        ///
        Algorithm();

        /// Constructs instance of algorithm class with movie
        /// type = SpMovie_t
        Algorithm(SpMovie_t movie);

        /// Does this class have a valid movie object
        /// returns bool
        bool IsValid();

        /// Gets output filename
        /// returns string
        std::string GetOutputFileName();

        /// Sets output filename
        /// type = string
        void SetOutputFileName(std::string outputFile);

        /// Applies algorithm
        /// (currently DF/F)
        void ApplyApp();

        /// Finds min and max elements in an array
        /// type = double
        void FindMinMax(const std::vector<double> &input, double &min, double &max, size_t nLength);

        /// Scales doubles to uint_16
        /// type = double
        void ScaleValues16(const std::vector<double> &input, double &min, double &max, std::vector<uint16_t> &output, size_t nLength);

        /// Adds two vectors of equal length
        /// type = std::vector<unsigned int>
        void Add_V(const std::vector<double> &inputA, const std::vector<uint16_t> &inputB, std::vector<double> &output, size_t nLength);
        
        /// Adds two vectors of equal length
        /// type = std::vector<unsigned int>
        void Subtract_V(const std::vector<uint16_t> &inputA, const std::vector<double> &inputB, std::vector<double> &output, size_t nLength);
        
        /// Adds two vectors of equal length
        /// type = std::vector<unsigned int>
        void Divide_C(const std::vector<double> &inputA, double num, std::vector<double> &output, size_t nLength);
        
        /// Adds two vectors of equal length
        /// type = std::vector<unsigned int>
        void Divide_V(const std::vector<double> &inputA, const std::vector<double> &inputB, std::vector<double> &output, size_t nLength);

    private:
        /// Movie which the algorithm is affecting
        /// 
        SpMovie_t m_Movie;

        /// Output file name
        ///
        std::string m_OutputFileName;

    };
}