/**
 * @file 
 * @brief The brief description goes here.
 * 
 * The detailed description if any, goes here 
 * ******/

#include "moja/modules/cbm/cbmaggregatorcsvwriter.h"

#include <moja/flint/recordaccumulatorwithmutex.h>
#include <moja/flint/ilandunitdatawrapper.h>
#include <moja/flint/iflintdata.h>
#include <moja/flint/ivariable.h>
#include <moja/logging.h>
#include <moja/signals.h>
#include <moja/notificationcenter.h>
#include <moja/hash.h>

#include <boost/algorithm/string.hpp>
#include <boost/format.hpp>

#include <Poco/Exception.h>
#include <Poco/File.h>
#include <Poco/FileStream.h>
#include <Poco/TeeStream.h>

namespace moja {
namespace modules {
namespace cbm {

     /**
     * @brief Constructor.
     * 
     * Initialise CBMFlatFile._tempPath to a temporary file path, as parameter path + "_" + random number generated using rand(), \n
     * CBMFlatFile._outputFile as make_unique<Poco::File>(_tempPath), \n
     * CBMFlatFile._streamFile as make_unique<Poco::FileOutputStream>(_tempPath), \n
     * CBMFlatFile. _outputStream as make_unique<Poco::TeeOutputStream>(*_streamFile)
     * 
     * The parameter is written in the _outputStream.
     * 
     * @param path string&
     * @param header string&
     * @return void
     * ************************/
    CBMFlatFile::CBMFlatFile(const std::string& path, const std::string& header) : _path(path) {
        _tempPath = (boost::format("%1%_%2%") % path % rand()).str();
        _outputFile = std::make_unique<Poco::File>(_tempPath);
        _outputFile->createFile();
        _streamFile = std::make_unique<Poco::FileOutputStream>(_tempPath);
        _outputStream = std::make_unique<Poco::TeeOutputStream>(*_streamFile);
        write(header);
    }

     /**
     * @brief Write parameter text to *_outputStream.
     * 
     * @param text string&
     * @return void
     * ************************/
    void CBMFlatFile::write(const std::string& text) {
        (*_outputStream) << text;
    }

     /**
     * @brief Save an existing file.
     * 
     * @return void
     * ************************/
    void CBMFlatFile::save() {
        _streamFile->close();
        _outputFile->renameTo(_path);
    }

     /**
     * @brief Configuration function.
     * 
     * Initialize CBMFlatFile._outputPath as config["output_path"] (string), \n 
     * If parameter config contains "separate_years", initialise CBMFlatFile._separateYears as config['separate_years'] (boolean) 
     * 
     * @param config DynamicObject&
     * @return void
     * ************************/
    void CBMAggregatorCsvWriter::configure(const DynamicObject& config) {
        _outputPath = config["output_path"].convert<std::string>();
        if (config.contains("separate_years")) {
            _separateYears = config["separate_years"].convert<bool>();
        }
    }

     /**
     * @brief Subscribe to the signals SystemInit, LocalDomainInit and SystemShutdown
     * 
     * @param notificationCenter NotificationCenter&
     * @return void
     * ************************/

    void CBMAggregatorCsvWriter::subscribe(NotificationCenter& notificationCenter) {
        notificationCenter.subscribe(signals::SystemInit,      &CBMAggregatorCsvWriter::onSystemInit,      *this);
        notificationCenter.subscribe(signals::LocalDomainInit, &CBMAggregatorCsvWriter::onLocalDomainInit, *this);
        notificationCenter.subscribe(signals::SystemShutdown,  &CBMAggregatorCsvWriter::onSystemShutdown,  *this);
	}

     /**
     * @brief Do System Init. 
     * 
     * If CBMAggregatorCsvWriter._isPrimaryAggregator is true, then
     * output directories are created
     * 
     * @param path string&
     * @param header string&
     * @return void
     * @exception FileExistsException&: if file already exists
     * ************************/
	void CBMAggregatorCsvWriter::doSystemInit() {
        if (!_isPrimaryAggregator) {
            return;
        }

        Poco::File outputDir(_outputPath);
        try {
            outputDir.createDirectories();
        } catch (Poco::FileExistsException&) { }
    }

     /**
     * @brief Initiate Local Domain.
     * 
     * Assign CBMAggregatorCsvWriter._jobId the value of variable "job_id" in _landUnitData, \n
     * if it exists, else set CBMAggregatorCsvWriter._jobId to 0
     * 
     * @return void
     * ************************/

    void CBMAggregatorCsvWriter::doLocalDomainInit() {
        _jobId = _landUnitData->hasVariable("job_id")
            ? _landUnitData->getVariable("job_id")->value().convert<Int64>()
            : 0;
    }

     /**
     * @brief Perform System Shut Down.
     * 
     * If CBMAggregatorCsvWriter._isPrimaryAggregator is true and if, \n
     * CBMAggregatorCsvWriter._classifierNames is not empty, \n
     * the flux, pool, error, age and disturbance data is loaded
     * 
     * @return void
     * ************************/
    void CBMAggregatorCsvWriter::doSystemShutdown() {
        if (!_isPrimaryAggregator) {
            return;
        }

        if (_classifierNames->empty()) {
			MOJA_LOG_INFO << "No data to load.";
			return;
		}

        load((boost::format("%1%/flux")        % _outputPath).str(), (boost::format("flux_%1%")        % _jobId).str(), _classifierNames, _fluxDimension);
        load((boost::format("%1%/pool")        % _outputPath).str(), (boost::format("pool_%1%")        % _jobId).str(), _classifierNames, _poolDimension);
        load((boost::format("%1%/error")       % _outputPath).str(), (boost::format("error_%1%")       % _jobId).str(), _classifierNames, _errorDimension);
        load((boost::format("%1%/age")         % _outputPath).str(), (boost::format("age_%1%")         % _jobId).str(), _classifierNames, _ageDimension);
        load((boost::format("%1%/disturbance") % _outputPath).str(), (boost::format("disturbance_%1%") % _jobId).str(), _classifierNames, _disturbanceDimension);

        MOJA_LOG_INFO << "Finished loading results." << std::endl;
    }

     /**
     * @brief Inserting Records
     * 
     * Assign variable records as dataDimension->records() \n
     * If records is empty, then return \n 
     * If CBMAggregatorCsvWriter._separateYears is false, create output directories and write each record to the output stream \n
     * Else write the records to the output stream grouped based on the year
     * 
     * @param outputPath string&
     * @param outputFilename string&
     * @param classifierNames shared_ptr<vector<string>>
     * @tparam dataDimension shared_ptr<TAccumulator>
     * @return void
     * @exception FileExistsException&: if the file already exists
     * ************************/

    template<typename TAccumulator>
    void CBMAggregatorCsvWriter::load(
        const std::string& outputPath,
        const std::string& outputFilename,
        std::shared_ptr<std::vector<std::string>> classifierNames,
        std::shared_ptr<TAccumulator> dataDimension) {


        MOJA_LOG_INFO << (boost::format("Loading %1%") % outputPath).str();

        auto records = dataDimension->records();
        if (records.empty()) {
            return;
        }

        if (!_separateYears) {
            Poco::File outputDir(outputPath);
            try {
                outputDir.createDirectories();
            } catch (Poco::FileExistsException&) {}

            auto csvOutputPath = (boost::format("%1%/%2%.csv") % outputPath % outputFilename).str();
            std::shared_ptr<CBMFlatFile> outputFile;
            for (auto& record : records) {
                if (outputFile == nullptr) {
                    outputFile = std::make_shared<CBMFlatFile>(csvOutputPath, record.header(*classifierNames));
                }

                outputFile->write(record.asPersistable());
            }

           outputFile->save();
        } else {
            std::unordered_map<int, std::shared_ptr<CBMFlatFile>> flatFiles;
            for (auto& record : records) {
                if (flatFiles.find(record.getYear()) == flatFiles.end()) {
                    auto yearOutputPath = (boost::format("%1%/%2%") % outputPath % record.getYear()).str();
                    Poco::File yearOutputDir(yearOutputPath);
                    try {
                        yearOutputDir.createDirectories();
                    } catch (Poco::FileExistsException&) {}

                    auto yearOutputFilename = (boost::format("%1%/%2%_%3%.csv")
                        % yearOutputPath % outputFilename % record.getYear()).str();

                    flatFiles[record.getYear()] = std::make_shared<CBMFlatFile>(
                        yearOutputFilename, record.header(*classifierNames));
                }

                flatFiles[record.getYear()]->write(record.asPersistable());
            }

            for (auto& flatFile : flatFiles) {
                flatFile.second->save();
            }
        }
    }

}}} // namespace moja::modules::cbm
