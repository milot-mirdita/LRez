#include "subcommands/compare.h"
#include "subcommands/help.h"
#include "barcodesComparison.h"
#include "indexManagementBam.h"
#include <getopt.h>

void subcommandCompare(int argc, char* argv[]) {
	string bamFile;
	string indexFile;
	string regionsFile;
	string outputFile;
	string contig;
	unsigned size = 1000;
	ofstream out;
	BamRegion r;

	const struct option longopts[] = {
		{"bam",			required_argument,	0, 'b'},
		{"regions",		required_argument,	0, 'r'},
		{"contig",		required_argument,	0, 'c'},
		{"index",		required_argument,	0, 'i'},
		{"size",		required_argument,	0, 's'},
		{"output",		required_argument,	0, 'o'},
		{"userx",			no_argument,	0, 'u'},
		{0, 0, 0, 0},
	};
	int index;
	int iarg = 0;

	iarg = getopt_long(argc, argv, "b:r:c:i:s:o:u", longopts, &index);
	if (iarg == -1) {
		subcommandHelp("compare");
	}
	while (iarg != -1) {
		switch (iarg) {
			case 'b':
				bamFile = optarg;
				break;
			case 'r':
				regionsFile = optarg;
				break;
			case 'c':
				contig = optarg;
				break;
			case 'i':
				indexFile = optarg;
				break;
			case 's':
				size = stoul(optarg);
				break;
			case 'o':
				outputFile = optarg;
				break;
			case 'u':
				CONSIDER_RX = true;
				break;
			default:
				subcommandHelp("extract");
				break;
		}
		iarg = getopt_long(argc, argv, "b:r:c:i:s:o:u", longopts, &index);
	}

	if (bamFile.empty()) {
		fprintf(stderr, "Please specify a BAM file with option --bam/-b.\n");
		exit(EXIT_FAILURE);
	}
	if (regionsFile.empty() and contig.empty()) {
		fprintf(stderr, "Please specify a file containg a list of regions with option --region/-r, specify a contig of interest with option --contig/-c.\n");
		exit(EXIT_FAILURE);
	}
	if (!regionsFile.empty() and !contig.empty()) {
		fprintf(stderr, "Options --regions/-r and --contig/-c cannot be used at the same time.\n");
		exit(EXIT_FAILURE);
	}
	if (!contig.empty() and indexFile.empty()) {
		fprintf(stderr, "Please specify an index file with option --index/-i.\n");
		exit(EXIT_FAILURE);
	}

	robin_hood::unordered_map<pair<string, string>, unsigned, hashPairs> commonBarcodes;
	// Compare the barcodes of all regions provided in the file with each other and output the result "matrix"
	if (!regionsFile.empty()) {
		commonBarcodes = compareRegions(bamFile, regionsFile);
	} else {
		// Compare the barcodes contained in the extremities of the provided contig with the barcodes contained in the extremities of other contigs
		BarcodesOffsetsIndex BarcodesOffsetsIndex;
		BarcodesOffsetsIndex = loadBarcodesOffsetsIndex(indexFile);
		commonBarcodes = compareContig(bamFile, BarcodesOffsetsIndex, contig, size);
	}

	if (!outputFile.empty()) {
		out.open(outputFile, ios::out);
		if (!out.is_open()) {
			fprintf(stderr, "Unable to open file %s.", outputFile.c_str());
			exit(EXIT_FAILURE);
		}
		for (auto c : commonBarcodes) {
			out << c.first.first << " " << c.first.second << " " << c.second << endl;
		}
		out.close();
	} else {
		for (auto c : commonBarcodes) {
			cout << c.first.first << " " << c.first.second << " " << c.second << endl;
		}
	}
}
