#include "logger.h"
#include "basic.h"
using namespace std;

namespace qram_simulator {

	map<string, profile*> profiler::profiles;
	bool profiler::on = true;
	Logger logger;

	string _datetime() {
		char buf[80];
		time_t now = time(0);
		tm tstruct;
#ifdef WIN32
		localtime_s(&tstruct, &now);
#else
		localtime_r(&now, &tstruct);
#endif
		strftime(buf, sizeof(buf), "%Y-%m-%d %H:%M:%S", &tstruct);
		return buf;
	}

	string _datetime_simple() {
		char buf[80];
		time_t now = time(0);
		tm tstruct;
#ifdef WIN32
		localtime_s(&tstruct, &now);
#else
		localtime_r(&now, &tstruct);
#endif
		strftime(buf, sizeof(buf), "%Y%m%d-%H%M%S", &tstruct);
		return buf;
	}

	void Logger::newfile_auto() {
		string autofname = autofilename("TestResult ", ".txt");
		out = ofstream(autofname);
	}

	string profiler::get_all_profiles_v2() {
		string ret;
		if (profiles.empty()) {
			return "No profiles.";
		}
		else {
			ret += fmt::format("Item: {}\n", profiles.size());
		}
		vector<pair<void*, void*>> profvec;
		map2vec(profvec, profiles);

		auto get_time = [](const pair<void*, void*>& item) {
			return (*static_cast<profile**>(item.second))->time;
		};
		auto get_name = [](const pair<void*, void*>& item) {
			return (*static_cast<string*>(item.first));
		};
		auto get_profile = [](const pair<void*, void*>& item) {
			return (*static_cast<profile**>(item.second));
		};

		sort(profvec.begin(), profvec.end(), [&get_time](
			const pair<void*, void*>& item1,
			const pair<void*, void*>& item2)
			{
				return get_time(item1) > get_time(item2);
			}
		);

		for (const auto& profile : profvec) {
			ret += fmt::format("[{:^38s}] Calls = {:^3d} Time = {:^4f} ms\n",
				get_name(profile)/*truncate_name(get_name(profile), 25)*/,
				get_profile(profile)->ncalls, get_profile(profile)->time);
		}
		return ret;
	}
} // namespace qram_simulator