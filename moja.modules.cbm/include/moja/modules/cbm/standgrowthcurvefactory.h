#ifndef MOJA_MODULES_CBM_MOSSPARAMETERS_H_
#define MOJA_MODULES_CBM_MOSSPARAMETERS_H_

#include "moja/modules/cbm/_modules.cbm_exports.h"
#include "moja/flint/modulebase.h"

#include "moja/modules/cbm/standgrowthcurve.h"
#include "tbb/concurrent_unordered_map.h"

namespace moja {
namespace modules {
namespace cbm {

	/// <summary>
	/// Singlenton factory class to create a stand growth curve.
	/// This object will be instantiated in module factory, and be 
	/// injected to other objects that requires the stand growth factory.
	/// </summary>	
	class CBM_API StandGrowthCurveFactory {

	public:
		StandGrowthCurveFactory();
		virtual ~StandGrowthCurveFactory() = default;

		std::shared_ptr<StandGrowthCurve> createStandGrowthCurve(Int64 standGrowthCurveID, int spuID, flint::ILandUnitDataWrapper& landUnitData);		

		std::shared_ptr<StandGrowthCurve> getStandGrowthCurve(Int64 growthCurveID);
	private:
		//for each stand growth curve, the yield volume is not changed by SPU
		//just create a thread sage lookup map by stand growth curve ID 		
		tbb::concurrent_unordered_map<Int64, std::shared_ptr<StandGrowthCurve>> _standGrowthCurves;

		void addStandGrowthCurve(Int64 standGrowthCurveID, std::shared_ptr<StandGrowthCurve>);
	};
}}}
#endif