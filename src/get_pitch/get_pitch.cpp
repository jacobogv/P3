/// @file

#include <iostream>
#include <fstream>
#include <string.h>
#include <errno.h>

#include "wavfile_mono.h"
#include "pitch_analyzer.h"

#include "docopt.h"

#define FRAME_LEN   0.030 /* 30 ms. */
#define FRAME_SHIFT 0.015 /* 15 ms. */

using namespace std;
using namespace upc;

void lowPassFilter(std::vector<float>& signal, int window_size = 5) {
  std::vector<float> filtered_signal(signal.size(), 0.0f);

  for (size_t i = window_size; i < signal.size() - window_size; ++i) {
      float sum = 0.0f;

      // Apply moving average filter by averaging over the window
      for (int j = -window_size; j <= window_size; ++j) {
          sum += signal[i + j];
      }

      filtered_signal[i] = sum / (2 * window_size + 1);
  }

  // Replace the original signal with the filtered signal
  signal = filtered_signal;
}

#include <algorithm>  // For std::nth_element
#include <deque>

void medianFilter(std::vector<float>& f0, int window_size = 5) {
    std::deque<float> window;
    for (size_t i = 0; i < f0.size(); ++i) {
        window.push_back(f0[i]);
        
        // If the window exceeds the specified size, remove the oldest value
        if (window.size() > window_size) {
            window.pop_front();
        }

        // Only apply median filter after the window is fully populated
        if (window.size() == window_size) {
            // Create a sorted copy of the window
            std::vector<float> sorted_window(window.begin(), window.end());
            std::nth_element(sorted_window.begin(), sorted_window.begin() + sorted_window.size() / 2, sorted_window.end());
            f0[i] = sorted_window[sorted_window.size() / 2];  // Replace the current element with the median
        }
    }
}

static const char USAGE[] = R"(
get_pitch - Pitch Estimator 

Usage:
    get_pitch [options] <input-wav> <output-txt>
    get_pitch (-h | --help)
    get_pitch --version

Options:
    -h, --help  Show this screen
    --version   Show the version of the project
    --window=<win>      Window type for pitch analysis [default: RECT]. Choices: RECT, HAMMING
    --low-pass-filter    Apply low-pass filtering to the signal


Arguments:
    input-wav   Wave file with the audio signal
    output-txt  Output file: ASCII file with the result of the estimation:
                    - One line per frame with the estimated f0
                    - If considered unvoiced, f0 must be set to f0 = 0
)";

int main(int argc, const char *argv[]) {
	/// \TODO 
	///  Modify the program syntax and the call to **docopt()** in order to
	///  add options and arguments to the program.
    std::map<std::string, docopt::value> args = docopt::docopt(USAGE,
        {argv + 1, argv + argc},	// array of arguments, without the program name
        true,    // show help if requested
        "2.0");  // version string

	std::string input_wav = args["<input-wav>"].asString();
	std::string output_txt = args["<output-txt>"].asString();
  std::string window_type = args["--window"].asString();
  bool low_pass_filter = args["--low-pass-filter"].asBool();

  // Read input sound file
  unsigned int rate;
  vector<float> x;
  if (readwav_mono(input_wav, rate, x) != 0) {
    cerr << "Error reading input file " << input_wav << " (" << strerror(errno) << ")\n";
    return -2;
  }

  int n_len = rate * FRAME_LEN;
  int n_shift = rate * FRAME_SHIFT;

  // Define analyzer
  PitchAnalyzer analyzer(n_len, rate, PitchAnalyzer::RECT, 50, 500);

  /// \TODO
  /// Preprocess the input signal in order to ease pitch estimation. For instance,
  /// central-clipping or low pass filtering may be used.
  if(low_pass_filter){
    lowPassFilter(x);
  }
    
  // Iterate for each frame and save values in f0 vector
  vector<float>::iterator iX;
  vector<float> f0;
  for (iX = x.begin(); iX + n_len < x.end(); iX = iX + n_shift) {
    float f = analyzer(iX, iX + n_len);
    f0.push_back(f);
  }

  /// \TODO
  /// Postprocess the estimation in order to supress errors. For instance, a median filter
  /// or time-warping may be used.
  medianFilter(x);

  // Write f0 contour into the output file
  ofstream os(output_txt);
  if (!os.good()) {
    cerr << "Error reading output file " << output_txt << " (" << strerror(errno) << ")\n";
    return -3;
  }

  os << 0 << '\n'; //pitch at t=0
  for (iX = f0.begin(); iX != f0.end(); ++iX) 
    os << *iX << '\n';
  os << 0 << '\n';//pitch at t=Dur

  return 0;
}
