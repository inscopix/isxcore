#ifndef ISX_VESSEL_CORRELATIONS_H
#define ISX_VESSEL_CORRELATIONS_H

#include "isxTimingInfo.h"
#include "isxSpacingInfo.h"
#include "isxCoreFwd.h"
#include "isxImage.h"
#include "isxTrace.h"

#include <limits>

namespace isx
{
    /// Correlation heatmap triptychs for single velocity measurement
    class VesselCorrelations
    {
        public:
            /// Default constructor
            ///
            VesselCorrelations();

            /// Constructor
            /// \param inSizeInPixels   Size of heatmap in pixels
            /// \param inMin            Min of heatmaps (saved in metadata)
            /// \param inMax            Max of heatmaps (saved in metadata)
            ///
            VesselCorrelations(
                const SizeInPixels_t inSizeInPixels,
                const float inMin = std::numeric_limits<float>::max(),
                const float inMax = std::numeric_limits<float>::min());

            /// \return The number of pixels.
            ///
            const SizeInPixels_t & getNumPixels() const;

            /// \return The total number of pixels.
            ///
            size_t getTotalNumPixels() const;

            /// \return a raw pointer to the first sample in memory
            ///
            float *
            getValues();

            /// \return a raw pointer to the first sample in memory of a specified correlation heatmap
            /// \param inOffset     The temporal offset of the correlation heatmap (-1, 0, +1)
            ///
            float *
            getValues(const int inOffset);

            /// Sets heatmap data for a specified temporal offset
            /// \param inOffset     The temporal offset of the correlation heatmap (-1, 0, +1)
            /// \param inData       The image data for the correlation heatmap
            ///
            void setValues(const int inOffset, const float * inData);

            /// Get the heatmap image for a specified temporal offset
            /// \param inOffset     The temporal offset of the correlation heatmap (-1, 0, +1)
            /// \return the heatmap image
            ///
            SpImage_t getHeatmap(const int inOffset);
            
            /// Get the heatmap image for a specified temporal offset
            /// \return the heatmap image
            ///
            SpImage_t getHeatmaps();

            /// calculates min max of movie
            void calculateMinMax();

            /// returns min of triptychs
            float getMin();

            /// returns max of triptychs
            float getMax();

        private:
            std::unique_ptr<float[]> m_values = 0;          ///< The correlation values serialized
            SizeInPixels_t m_sizeInPixels;                  ///< The size of a single correlation heatmap
            float m_min = std::numeric_limits<float>::max();
            float m_max = std::numeric_limits<float>::min();
    };
    using SpVesselCorrelations_t = std::shared_ptr<VesselCorrelations>;

    /// Trace of correlation heatmap triptychs for trace of velocity measurements
    /// Note: use setValue instead of memcpy to ensure shared pointers are updated correctly
    class VesselCorrelationsTrace : public Trace<SpVesselCorrelations_t>
    {
        public:
            /// Constructor
            /// \param inTimingInfo     Timing info of the trace
            /// \param inSizeInPixels   Size of heatmap in pixels
            VesselCorrelationsTrace(
                const TimingInfo & inTimingInfo,
                SizeInPixels_t inSizeInPixels)
                : Trace<SpVesselCorrelations_t>(inTimingInfo)
            {
                for (size_t i = 0; i < inTimingInfo.getNumTimes(); i++)
                {
                    setValue(i, std::make_shared<VesselCorrelations>(inSizeInPixels));
                }
            }
    };
    using SpVesselCorrelationsTrace_t = std::shared_ptr<VesselCorrelationsTrace>;

    /// Map coordinates of box from movie to minimum cartesian rectangle enclosing box
    /// \param inBoundingBox                The bounding box drawn on a movie
    ///
    Contours_t mapBoxToMinimunRect(const Contour_t & inBoundingBox);
    
    /// Computes the center of a bounding box
    /// \param inBoundingBox                The bounding box of a vessel         
    ///
    PointInPixels_t computeCenterOfBoundingBox(const Contour_t & inBoundingBox);

    /// Calculates the size of a correlation heatmap triptych
    /// Heatmaps are arranged row-wise or column-wise
    /// depending on the dimensions of a single correlation heatmap
    /// \param inCorrelationSize            The size of a single correlation heatmap
    ///
    SizeInPixels_t computeTriptychSize(const SizeInPixels_t & inCorrelationSize);

} // namespace isx

#endif // ISX_VESSEL_CORRELATIONS_H
