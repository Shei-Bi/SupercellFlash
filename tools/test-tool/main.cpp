#include <flash/flash.h>
#include <core/time/timer.h>
#include <string>

#include <iostream>
#include <chrono>

using namespace std;
using namespace std::chrono;
using namespace sc::flash;
using namespace wk;

namespace fs = std::filesystem;

int main(int argc, char* argv[])
{
	SupercellSWF swf1;
	SupercellSWF swf2;

	swf1 = swf2;

	if (argc <= 1) {
		return 1;
	}

	fs::path filepath = argv[1];
	if (!fs::exists(filepath)) {
		cout << "File not found";
		return 1;
	}

	/* Loading test */
	time_point loading_start = high_resolution_clock::now();
	Timer loading;
	SupercellSWF swf;
	swf.load(filepath);

	cout << "Loading took: ";
	cout << loading.elapsed() << endl << endl;
	loading.reset();

	// swf.GetDisplayObjectByID(10623)
	for (sc::flash::ExportName& n : swf.exports) {
		if (n.name == "event_icon_invasion") {
			sc::flash::MovieClip* mc = (sc::flash::MovieClip*)&swf.GetDisplayObjectByID(n.id);
			for (DisplayObjectInstance& doi : mc->childrens) {
				cout << n.name.string() << " " << doi.id << endl;
			}
		}
		// cout << n.name.string() << " " << n.id << endl;
	}
	// /* Save test */
	// fs::path folder = filepath.parent_path();
	// try {
	// 	swf.save(folder / filepath.stem().concat("_new").concat(filepath.extension().string()), Signature::Zstandard);
	// }
	// catch (const wk::Exception& err) {
	// 	cout << "Error. " << endl << "Message: " << err.what() << endl;
	// }

	// cout << "Saving took: ";
	// cout << loading.elapsed() << endl;

	return 0;
}