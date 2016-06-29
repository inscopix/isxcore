#pragma once

#include <string>
#include <vector>
#include <memory>
#include "isxCoreFwd.h"
#include "isxAsyncTaskHandle.h"

namespace isx {

    ///
    /// A class encapsulating an nVista movie file.
    ///
    class DeltaFoverF
    {
    public:
        /// Default constructor.  Is a valid C++ object but not usable.
        ///
        DeltaFoverF();

        /// Constructs instance of algorithm class with movie
        /// type = SpMovie_t
        DeltaFoverF(const SpMovie_t & movie);

        /// Does this class have a valid movie object
        /// returns bool
        bool isValid();

        /// Gets output movie
        /// \return The output Movie
        const SpMovie_t & getOutputMovie() const;

        /// Sets output filename
        /// type = string
        void setOutputMovie(const SpMovie_t & inMovie);

        /// Applies DF/F
        /// \param inCheckInCB callback to invoke periodically
        AsyncTaskFinishedStatus run(AsyncTaskHandle::CheckInCB_t inCheckInCB);

        /// Finds min and max elements in an array
        /// type = double
        void findMinMax(const std::vector<double> &input, double &min, double &max, size_t nLength);

        /// Scales doubles to uint_16
        /// type = double
        void scaleValues16(const std::vector<double> &input, double &min, double &max, std::vector<uint16_t> &output, size_t nLength);

        /// Adds two vectors of equal length: output = inputA + inputB
        /// type = std::vector<unsigned int>
        void add_V(const std::vector<double> &inputA, uint16_t * inputB, std::vector<double> &output, size_t nLength);
        
        /// Subtracts two vectors of equal length: output = inputA - inputB
        /// type = std::vector<unsigned int>
        void subtract_V(uint16_t * inputA, const std::vector<double> &inputB, std::vector<double> &output, size_t nLength);
        
        /// Divides elements of a vector by a scalar: output = inputA / num
        /// type = std::vector<unsigned int>
        void divide_C(const std::vector<double> &inputA, double num, std::vector<double> &output, size_t nLength);
        
        /// Divides two vectors of equal length: output = inputA / inputB
        /// type = std::vector<unsigned int>
        void divide_V(const std::vector<double> &inputA, const std::vector<double> &inputB, std::vector<double> &output, size_t nLength);

    private:
        /// Movie which the algorithm is affecting
        /// 
        SpMovie_t m_movie;

        /// Output Movie
        ///
        SpMovie_t m_outputMovie;

        AsyncTaskHandle::CheckInCB_t m_checkInCB;

    };
}