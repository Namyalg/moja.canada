#ifndef results_h
#define results_h

struct Sawtooth_Matrix {
	//number of rows in the matrix
	int rows;
	//number of columns in the matrix
	int cols;
	//matrix storage
	double* values;
	//total number of elements in storage
	int size() { return rows*cols; }

	double GetValue(int row, int col) {
		return values[row * cols + col];
	}

	void SetValue(int row, int col, double value) {
		values[row * cols + col] = value;
	}
};

//tracks tree level results by timestep, by tree index
//one TreeLevelResult struct instance is required per stand
//all matrices are by step number (rows), by tree index (cols)
struct TreeLevelResult {
	//age (years)
	Sawtooth_Matrix* Age;
	//tree height (m)
	Sawtooth_Matrix* Height;
	//aboveground live Carbon (kg C)
	Sawtooth_Matrix* C_AG;
	//aboveground Carbon growth (kg C yr-1)
	Sawtooth_Matrix* C_AG_G;
	//live 1, or dead 0
	Sawtooth_Matrix* Live;
	//recruitment (if value=1, otherwise 0)
	Sawtooth_Matrix* Recruitment;
	//aboveground live Carbon losses due to annual mortality, and 
	//self-thinning (kg C)
	Sawtooth_Matrix* Mortality_C_ag;
	//regular or thinning mortality (if value=1, otherwise 0)
	Sawtooth_Matrix* MortalityCode;
	//aboveground live Carbon losses due to disturbance (kg C)
	Sawtooth_Matrix* Disturbance_C_ag;
	//the disturbance type that occurred for specified trees/timesteps
	Sawtooth_Matrix* DisturbanceType;
};

//tracks stand level aggregate results by timestep (rows) by stand (cols)
//one struct instance is required for the entire simulation space 
struct StandLevelResult {
	Sawtooth_Matrix* MeanAge;
	Sawtooth_Matrix* MeanHeight;
	// stand density (stems ha^-1)
	Sawtooth_Matrix* StandDensity;
	// total biomass carbon (Mg C ha-1)
	Sawtooth_Matrix* TotalBiomassCarbon;
	// total biomass carbon growth (Mg C ha-1 yr-1)
	Sawtooth_Matrix* TotalBiomassCarbonGrowth;
	// mean biomass carbon (kg C)
	Sawtooth_Matrix* MeanBiomassCarbon;
	// Demographic recruitment rate (% yr-1)
	Sawtooth_Matrix* RecruitmentRate;
	// Demographic mortality rate (% yr-1) (regular mortality and thinning)
	Sawtooth_Matrix* MortalityRate;
	// Total mortality Carbon (Mg C ha-1 yr-1) (regular mortality and thinning)
	Sawtooth_Matrix* MortalityCarbon;
	// Disturbance type code 
	Sawtooth_Matrix* DisturbanceType;
	// Demographic mortality rate (% yr-1) (disturbance)
	Sawtooth_Matrix* DisturbanceMortalityRate;
	// Total disturbance mortality (Mg C ha-1 yr-1)
	Sawtooth_Matrix* DisturbanceMortalityCarbon;
};

#endif // 
