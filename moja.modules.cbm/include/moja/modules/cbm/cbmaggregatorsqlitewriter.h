#ifndef MOJA_MODULES_CBM_CBMAGGREGATORSQLITEWRITER_H_
#define MOJA_MODULES_CBM_CBMAGGREGATORSQLITEWRITER_H_

#include "moja/modules/cbm/_modules.cbm_exports.h"
#include "moja/modules/cbm/record.h"
#include "moja/flint/recordaccumulatortbb.h"
#include "moja/flint/modulebase.h"
#include "moja/flint/ipool.h"
#include "moja/flint/spatiallocationinfo.h"
#include "moja/hash.h"

#include <Poco/Tuple.h>

#include <fstream>
#include <iostream>
#include <vector>
#include <string>
#include <unordered_map>
#include <functional>
#include <set>

namespace moja {
namespace modules {
namespace cbm {

    class CBM_API CBMAggregatorSQLiteWriter : public flint::ModuleBase {
    public:
        CBMAggregatorSQLiteWriter(
            std::shared_ptr<flint::RecordAccumulatorTBB<DateRow>> dateDimension,
            std::shared_ptr<flint::RecordAccumulatorTBB<PoolInfoRow>> poolInfoDimension,
            std::shared_ptr<flint::RecordAccumulatorTBB<ClassifierSetRow>> classifierSetDimension,
            std::shared_ptr<flint::RecordAccumulatorTBB<LandClassRow>> landClassDimension,
            std::shared_ptr<flint::RecordAccumulatorTBB<TemporalLocationRow>> locationDimension,
            std::shared_ptr<flint::RecordAccumulatorTBB<ModuleInfoRow>> moduleInfoDimension,
            std::shared_ptr<std::set<std::string>> classifierNames,
            std::shared_ptr<flint::RecordAccumulatorTBB<PoolRow>> poolDimension,
            std::shared_ptr<flint::RecordAccumulatorTBB<FluxRow>> fluxDimension,
            bool isPrimary = false)
        : ModuleBase(),
          _dateDimension(dateDimension),
          _poolInfoDimension(poolInfoDimension),
          _classifierSetDimension(classifierSetDimension),
          _landClassDimension(landClassDimension),
          _locationDimension(locationDimension),
          _moduleInfoDimension(moduleInfoDimension),
          _classifierNames(classifierNames),
          _poolDimension(poolDimension),
          _fluxDimension(fluxDimension),
          _isPrimaryAggregator(isPrimary) {}

        virtual ~CBMAggregatorSQLiteWriter() = default;

        void configure(const DynamicObject& config) override;
        void subscribe(NotificationCenter& notificationCenter) override;

        flint::ModuleTypes moduleType() override { return flint::ModuleTypes::System; };

        void onSystemShutdown() override;

    private:
        std::shared_ptr<flint::RecordAccumulatorTBB<DateRow>> _dateDimension;
        std::shared_ptr<flint::RecordAccumulatorTBB<PoolInfoRow>> _poolInfoDimension;
        std::shared_ptr<flint::RecordAccumulatorTBB<ClassifierSetRow>> _classifierSetDimension;
        std::shared_ptr<flint::RecordAccumulatorTBB<LandClassRow>> _landClassDimension;
        std::shared_ptr<flint::RecordAccumulatorTBB<TemporalLocationRow>> _locationDimension;
        std::shared_ptr<flint::RecordAccumulatorTBB<ModuleInfoRow>> _moduleInfoDimension;
        std::shared_ptr<flint::RecordAccumulatorTBB<PoolRow>> _poolDimension;
        std::shared_ptr<flint::RecordAccumulatorTBB<FluxRow>> _fluxDimension;
        std::shared_ptr<std::set<std::string>> _classifierNames;

        std::string _dbName;
        bool _isPrimaryAggregator;
    };

}}} // namespace moja::modules::cbm

#endif // MOJA_MODULES_CBM_CBMAGGREGATORSQLITEWRITER_H_