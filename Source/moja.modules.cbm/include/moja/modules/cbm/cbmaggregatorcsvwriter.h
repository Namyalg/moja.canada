#ifndef MOJA_MODULES_CBM_CBMAGGREGATORCSVWRITER_H_
#define MOJA_MODULES_CBM_CBMAGGREGATORCSVWRITER_H_

#include "moja/modules/cbm/_modules.cbm_exports.h"
#include "moja/modules/cbm/flatrecord.h"
#include "moja/modules/cbm/cbmmodulebase.h"

#include <moja/flint/spatiallocationinfo.h>

#include <vector>

namespace Poco {
    class File;
    class FileOutputStream;
    class TeeOutputStream;
}

namespace moja {
namespace flint {
	template<class TPersistable, class TRecord>
	class RecordAccumulatorWithMutex2;
}

namespace modules {
namespace cbm {

    class CBMFlatFile {
    public:
        CBMFlatFile(const std::string& path, const std::string& header);

        virtual ~CBMFlatFile() = default;

        void write(const std::string& text);
        void save();

    private:
        std::string _path;
        std::string _tempPath;
        std::unique_ptr<Poco::File> _outputFile;
        std::unique_ptr<Poco::FileOutputStream> _streamFile;
        std::unique_ptr<Poco::TeeOutputStream> _outputStream;
    };

    class CBM_API CBMAggregatorCsvWriter : public CBMModuleBase {
    public:
        CBMAggregatorCsvWriter(
            std::shared_ptr<flint::RecordAccumulatorWithMutex2<std::string, FlatFluxRecord>> fluxDimension,
            std::shared_ptr<flint::RecordAccumulatorWithMutex2<std::string, FlatPoolRecord>> poolDimension,
            std::shared_ptr<flint::RecordAccumulatorWithMutex2<std::string, FlatErrorRecord>> errorDimension,
            std::shared_ptr<flint::RecordAccumulatorWithMutex2<std::string, FlatAgeAreaRecord>> ageDimension,
            std::shared_ptr<flint::RecordAccumulatorWithMutex2<std::string, FlatDisturbanceRecord>> disturbanceDimension,
			std::shared_ptr<std::vector<std::string>> classifierNames,
			bool isPrimary = false)
            : CBMModuleBase(),
              _fluxDimension(fluxDimension),
              _poolDimension(poolDimension),
              _errorDimension(errorDimension),
              _ageDimension(ageDimension),
              _disturbanceDimension(disturbanceDimension),
              _classifierNames(classifierNames),
              _isPrimaryAggregator(isPrimary),
              _separateYears(false) {}

        virtual ~CBMAggregatorCsvWriter() = default;

        void configure(const DynamicObject& config) override;
        void subscribe(NotificationCenter& notificationCenter) override;

        flint::ModuleTypes moduleType() override { return flint::ModuleTypes::System; };

		void doSystemInit() override;
        void doLocalDomainInit() override;
        void doSystemShutdown() override;

    private:
        std::shared_ptr<flint::RecordAccumulatorWithMutex2<std::string, FlatFluxRecord>> _fluxDimension;
        std::shared_ptr<flint::RecordAccumulatorWithMutex2<std::string, FlatPoolRecord>> _poolDimension;
        std::shared_ptr<flint::RecordAccumulatorWithMutex2<std::string, FlatErrorRecord>> _errorDimension;
        std::shared_ptr<flint::RecordAccumulatorWithMutex2<std::string, FlatAgeAreaRecord>> _ageDimension;
        std::shared_ptr<flint::RecordAccumulatorWithMutex2<std::string, FlatDisturbanceRecord>> _disturbanceDimension;
        std::shared_ptr<std::vector<std::string>> _classifierNames;

        std::shared_ptr<const flint::SpatialLocationInfo> _spatialLocationInfo;

        std::string _outputPath;
        Int64 _jobId;
        bool _isPrimaryAggregator;
        bool _separateYears;

        template<typename TAccumulator>
        void load(const std::string& outputPath,
                  const std::string& outputFilename,
                  std::shared_ptr<std::vector<std::string>> classifierNames,
                  std::shared_ptr<TAccumulator> dataDimension);
    };

}}} // namespace moja::modules::cbm

#endif // MOJA_MODULES_CBM_CBMAGGREGATORCSVWRITER_H_
