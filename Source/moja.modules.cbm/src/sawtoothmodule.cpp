#include <moja/notificationcenter.h>
#include <moja/signals.h>
#include <moja/flint/variable.h>

#include "moja/modules/cbm/sawtoothmodule.h"

namespace moja {
namespace modules {
namespace cbm {

	void SawtoothModule::configure(const DynamicObject& config) {

		Sawtooth_Error sawtooth_error;
		std::string sawtoothDbPath = config["sawtooth_db_path"];
		unsigned long long random_seed = config["random_seed"];
		Sawtooth_Max_Density = config["max_density"];
		
		speciesList = SawtoothMatrixWrapper<Sawtooth_Matrix_Int, int>(1, Sawtooth_Max_Density, 0);

		tmin_mat = SawtoothMatrixWrapper<Sawtooth_Matrix, double>(1, 1);
		tmean_mat = SawtoothMatrixWrapper<Sawtooth_Matrix, double>(1, 1);
		vpd_mat = SawtoothMatrixWrapper<Sawtooth_Matrix, double>(1, 1);
		etr_mat = SawtoothMatrixWrapper<Sawtooth_Matrix, double>(1, 1);
		eeq_mat = SawtoothMatrixWrapper<Sawtooth_Matrix, double>(1, 1);
		ws_mat = SawtoothMatrixWrapper<Sawtooth_Matrix, double>(1, 1);
		ca_mat = SawtoothMatrixWrapper<Sawtooth_Matrix, double>(1, 1);
		ndep_mat = SawtoothMatrixWrapper<Sawtooth_Matrix, double>(1, 1);
		ws_mjjas_z_mat = SawtoothMatrixWrapper<Sawtooth_Matrix, double>(1, 1);
		ws_mjjas_n_mat = SawtoothMatrixWrapper<Sawtooth_Matrix, double>(1, 1);
		etr_mjjas_z_mat = SawtoothMatrixWrapper<Sawtooth_Matrix, double>(1, 1);
		etr_mjjas_n_mat = SawtoothMatrixWrapper<Sawtooth_Matrix, double>(1, 1);
		disturbance_mat = SawtoothMatrixWrapper<Sawtooth_Matrix_Int, int>(1, 1);

		spatialVar.tmin = *tmin_mat.Get();
		spatialVar.tmean = *tmean_mat.Get();
		spatialVar.vpd = *vpd_mat.Get();
		spatialVar.etr = *etr_mat.Get();
		spatialVar.eeq = *eeq_mat.Get();
		spatialVar.ws = *ws_mat.Get();
		spatialVar.ca = *ca_mat.Get();
		spatialVar.ndep= *ndep_mat.Get();
		spatialVar.ws_mjjas_z = *ws_mjjas_z_mat.Get();
		spatialVar.ws_mjjas_n = *ws_mjjas_n_mat.Get();
		spatialVar.etr_mjjas_z = *etr_mjjas_z_mat.Get();
		spatialVar.etr_mjjas_n = *etr_mjjas_n_mat.Get();
		spatialVar.disturbances = *disturbance_mat.Get();

		MeanAge_mat = SawtoothMatrixWrapper<Sawtooth_Matrix, double>(1, 1);
		MeanHeight_mat = SawtoothMatrixWrapper<Sawtooth_Matrix, double>(1, 1);
		StandDensity_mat = SawtoothMatrixWrapper<Sawtooth_Matrix, double>(1, 1);
		TotalBiomassCarbon_mat = SawtoothMatrixWrapper<Sawtooth_Matrix, double>(1, 1);
		TotalBiomassCarbonGrowth_mat = SawtoothMatrixWrapper<Sawtooth_Matrix, double>(1, 1);
		MeanBiomassCarbon_mat = SawtoothMatrixWrapper<Sawtooth_Matrix, double>(1, 1);
		RecruitmentRate_mat = SawtoothMatrixWrapper<Sawtooth_Matrix, double>(1, 1);
		MortalityRate_mat = SawtoothMatrixWrapper<Sawtooth_Matrix, double>(1, 1);
		MortalityCarbon_mat = SawtoothMatrixWrapper<Sawtooth_Matrix, double>(1, 1);
		DisturbanceType_mat = SawtoothMatrixWrapper<Sawtooth_Matrix, double>(1, 1);
		DisturbanceMortalityRate_mat = SawtoothMatrixWrapper<Sawtooth_Matrix, double>(1, 1);
		DisturbanceMortalityCarbon_mat = SawtoothMatrixWrapper<Sawtooth_Matrix, double>(1, 1);

		standLevelResult.MeanAge = MeanAge_mat.Get();
		standLevelResult.MeanHeight = MeanHeight_mat.Get();
		standLevelResult.StandDensity = StandDensity_mat.Get();
		standLevelResult.TotalBiomassCarbon = TotalBiomassCarbon_mat.Get();
		standLevelResult.TotalBiomassCarbonGrowth = TotalBiomassCarbonGrowth_mat.Get();
		standLevelResult.MeanBiomassCarbon = MeanBiomassCarbon_mat.Get();
		standLevelResult.RecruitmentRate = RecruitmentRate_mat.Get();
		standLevelResult.MortalityRate = MortalityRate_mat.Get();
		standLevelResult.MortalityCarbon = MortalityCarbon_mat.Get();
		standLevelResult.DisturbanceType = DisturbanceType_mat.Get();
		standLevelResult.DisturbanceMortalityRate = DisturbanceMortalityRate_mat.Get();
		standLevelResult.DisturbanceMortalityCarbon = DisturbanceMortalityCarbon_mat.Get();

		StumpParmeterId_mat = SawtoothMatrixWrapper<Sawtooth_Matrix_Int, int>(1, 1);
		RootParameterId_mat = SawtoothMatrixWrapper<Sawtooth_Matrix_Int, int>(1, 1);
		TurnoverParameterId_mat = SawtoothMatrixWrapper<Sawtooth_Matrix_Int, int>(1, 1);
		RegionId_mat = SawtoothMatrixWrapper<Sawtooth_Matrix_Int, int>(1, 1);

		cbmVariables.StumpParameterId = *StumpParmeterId_mat.Get();
		cbmVariables.RootParameterId = *RootParameterId_mat.Get();
		cbmVariables.TurnoverParameterId = *TurnoverParameterId_mat.Get();
		cbmVariables.RegionId = *RegionId_mat.Get();

		annualProcess = std::make_shared<Sawtooth_CBMAnnualProcesses>();
		cbmResult.Processes = annualProcess.get();

		Sawtooth_Handle = Sawtooth_Initialize(&sawtooth_error,
			sawtoothDbPath.c_str(), InitializeModelMeta(config), random_seed);

		if (sawtooth_error.Code != Sawtooth_NoError) {
			BOOST_THROW_EXCEPTION(moja::flint::SimulationError()
				<< moja::flint::Details(std::string(sawtooth_error.Message))
				<< moja::flint::LibraryName("moja.modules.cbm")
				<< moja::flint::ModuleName("sawtoothmodule"));
		}
	}

	Sawtooth_ModelMeta SawtoothModule::InitializeModelMeta(const DynamicObject& config) {
		Sawtooth_ModelMeta meta;

		std::string mortality_model = config["mortality_model"];
		if (mortality_model == "Sawtooth_MortalityNone") meta.mortalityModel = Sawtooth_MortalityNone;
		else if (mortality_model == "Sawtooth_MortalityConstant") meta.mortalityModel = Sawtooth_MortalityConstant;
		else if (mortality_model == "Sawtooth_MortalityDefault") meta.mortalityModel = Sawtooth_MortalityDefault;
		else if (mortality_model == "Sawtooth_MortalityES1") meta.mortalityModel = Sawtooth_MortalityES1;
		else if (mortality_model == "Sawtooth_MortalityES2") meta.mortalityModel = Sawtooth_MortalityES2;
		else if (mortality_model == "Sawtooth_MortalityMLR35") meta.mortalityModel = Sawtooth_MortalityMLR35;
		else BOOST_THROW_EXCEPTION(moja::flint::SimulationError()
			<< moja::flint::Details("specified sawtooth mortality_model not valid")
			<< moja::flint::LibraryName("moja.modules.cbm")
			<< moja::flint::ModuleName("sawtoothmodule"));

		std::string growth_model = config["growth_model"];
		if (growth_model == "Sawtooth_GrowthDefault") meta.growthModel = Sawtooth_GrowthDefault;
		else if (growth_model == "Sawtooth_GrowthES1") meta.growthModel = Sawtooth_GrowthES1;
		else if (growth_model == "Sawtooth_GrowthES2") meta.growthModel = Sawtooth_GrowthES2;
		else if (growth_model == "Sawtooth_GrowthES3") meta.growthModel = Sawtooth_GrowthES3;
		else BOOST_THROW_EXCEPTION(moja::flint::SimulationError()
			<< moja::flint::Details("specified sawtooth growth_model not valid")
			<< moja::flint::LibraryName("moja.modules.cbm")
			<< moja::flint::ModuleName("sawtoothmodule"));

		std::string recruitment_model = config["recruitment_model"];
		if (recruitment_model == "Sawtooth_RecruitmentDefault") meta.recruitmentModel = Sawtooth_RecruitmentDefault;
		else BOOST_THROW_EXCEPTION(moja::flint::SimulationError()
			<< moja::flint::Details("specified sawtooth recruitment_model not valid")
			<< moja::flint::LibraryName("moja.modules.cbm")
			<< moja::flint::ModuleName("sawtoothmodule"));

		return meta;
	}

	void SawtoothModule::subscribe(NotificationCenter& notificationCenter) {
		notificationCenter.connectSignal(signals::LocalDomainInit, &SawtoothModule::onLocalDomainInit, *this);
		notificationCenter.connectSignal(signals::TimingInit, &SawtoothModule::onTimingInit, *this);
		notificationCenter.connectSignal(signals::TimingStep, &SawtoothModule::onTimingStep, *this);
		notificationCenter.connectSignal(signals::TimingShutdown, &SawtoothModule::onTimingShutdown, *this);
		notificationCenter.connectSignal(signals::SystemShutdown, &SawtoothModule::onSystemShutdown, *this);
	}

	void SawtoothModule::doLocalDomainInit() {
		_softwoodStemSnag = _landUnitData->getPool("SoftwoodStemSnag");
		_softwoodBranchSnag = _landUnitData->getPool("SoftwoodBranchSnag");
		_softwoodMerch = _landUnitData->getPool("SoftwoodMerch");
		_softwoodFoliage = _landUnitData->getPool("SoftwoodFoliage");
		_softwoodOther = _landUnitData->getPool("SoftwoodOther");
		_softwoodCoarseRoots = _landUnitData->getPool("SoftwoodCoarseRoots");
		_softwoodFineRoots = _landUnitData->getPool("SoftwoodFineRoots");

		_hardwoodStemSnag = _landUnitData->getPool("HardwoodStemSnag");
		_hardwoodBranchSnag = _landUnitData->getPool("HardwoodBranchSnag");
		_hardwoodMerch = _landUnitData->getPool("HardwoodMerch");
		_hardwoodFoliage = _landUnitData->getPool("HardwoodFoliage");
		_hardwoodOther = _landUnitData->getPool("HardwoodOther");
		_hardwoodCoarseRoots = _landUnitData->getPool("HardwoodCoarseRoots");
		_hardwoodFineRoots = _landUnitData->getPool("HardwoodFineRoots");

		_aboveGroundVeryFastSoil = _landUnitData->getPool("AboveGroundVeryFastSoil");
		_aboveGroundFastSoil = _landUnitData->getPool("AboveGroundFastSoil");
		_belowGroundVeryFastSoil = _landUnitData->getPool("BelowGroundVeryFastSoil");
		_belowGroundFastSoil = _landUnitData->getPool("BelowGroundFastSoil");

		_mediumSoil = _landUnitData->getPool("MediumSoil");
		_atmosphere = _landUnitData->getPool("Atmosphere");

		_turnoverRates = _landUnitData->getVariable("turnover_rates");

		_regenDelay = _landUnitData->getVariable("regen_delay");

		_currentLandClass = _landUnitData->getVariable("current_land_class");

		auto rootParams = _landUnitData->getVariable("root_parameters")->value().extract<DynamicObject>();
		SWRootBio = std::make_shared<SoftwoodRootBiomassEquation>(
			rootParams["sw_a"], rootParams["frp_a"], rootParams["frp_b"], rootParams["frp_c"]);
		HWRootBio = std::make_shared<HardwoodRootBiomassEquation>(
			rootParams["hw_a"], rootParams["hw_b"], rootParams["frp_a"],
			rootParams["frp_b"], rootParams["frp_c"]);
	}

	void SawtoothModule::doTimingInit() {
		Sawtooth_Stand_Handle = Sawtooth_Stand_Alloc(&sawtooth_error, 1,
			Sawtooth_Max_Density, *speciesList.Get(), &cbmVariables );
		if (sawtooth_error.Code != Sawtooth_NoError) {
			BOOST_THROW_EXCEPTION(moja::flint::SimulationError()
				<< moja::flint::Details(std::string(sawtooth_error.Message))
				<< moja::flint::LibraryName("moja.modules.cbm")
				<< moja::flint::ModuleName("sawtoothmodule"));
		}
	}

	void SawtoothModule::doTimingStep() {

		tmin_mat.SetValue(1, 1, tmin->value());
		tmean_mat.SetValue(1, 1, tmean->value());
		vpd_mat.SetValue(1, 1, vpd->value());
		etr_mat.SetValue(1, 1, etr->value());
		eeq_mat.SetValue(1, 1, eeq->value());
		ws_mat.SetValue(1, 1, ws->value());
		ca_mat.SetValue(1, 1, ca->value());
		ndep_mat.SetValue(1, 1, ndep->value());
		ws_mjjas_z_mat.SetValue(1, 1, ws_mjjas_z->value());
		ws_mjjas_n_mat.SetValue(1, 1, ws_mjjas_n->value());
		etr_mjjas_z_mat.SetValue(1, 1, etr_mjjas_z->value());
		etr_mjjas_n_mat.SetValue(1, 1, etr_mjjas_n->value());
		disturbance_mat.SetValue(1, 1, disturbance->value());

		Sawtooth_Step(&sawtooth_error, Sawtooth_Handle, Sawtooth_Stand_Handle,
			1, spatialVar, &standLevelResult, NULL, &cbmResult);

		if (sawtooth_error.Code != Sawtooth_NoError) {
			BOOST_THROW_EXCEPTION(moja::flint::SimulationError()
				<< moja::flint::Details(std::string(sawtooth_error.Message))
				<< moja::flint::LibraryName("moja.modules.cbm")
				<< moja::flint::ModuleName("sawtoothmodule"));
		}
	}

	void SawtoothModule::doTimingShutdown() {
		Sawtooth_Stand_Free(&sawtooth_error, Sawtooth_Stand_Handle);
		if (sawtooth_error.Code != Sawtooth_NoError) {
			BOOST_THROW_EXCEPTION(moja::flint::SimulationError()
				<< moja::flint::Details(std::string(sawtooth_error.Message))
				<< moja::flint::LibraryName("moja.modules.cbm")
				<< moja::flint::ModuleName("sawtoothmodule"));
		}
	}

	void SawtoothModule::doSystemShutdown() {
		Sawtooth_Free(&sawtooth_error, Sawtooth_Handle);
		if (sawtooth_error.Code != Sawtooth_NoError) {
			BOOST_THROW_EXCEPTION(moja::flint::SimulationError()
				<< moja::flint::Details(std::string(sawtooth_error.Message))
				<< moja::flint::LibraryName("moja.modules.cbm")
				<< moja::flint::ModuleName("sawtoothmodule"));
		}
	}
}}}