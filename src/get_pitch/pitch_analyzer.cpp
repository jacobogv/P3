/// @file

#include <iostream>
#include <math.h>
#include "pitch_analyzer.h"

using namespace std;

/// Name space of UPC
namespace upc {
  void PitchAnalyzer::autocorrelation(const vector<float> &x, vector<float> &r) const {

    for (unsigned int l = 0; l < r.size(); ++l) {
  		/// \TODO Compute the autocorrelation r[l]
      /// Para cada TODO que hay en el código los completemos añadir comando
      /// \HECHO hemos hecho la autocorrelación sesgada
      /// \f[
      /// r_{xx}[m]=\frac{1}{N} \sum_{n=0}^{N-m} x[n] x[n+m]
      /// \f]
      r[l] = 0.0f;
      for (unsigned int n = 0; n < x.size() - l; ++n) {
        r[l] += x[n] * x[n+l];
      }
      r[l] /= x.size();//normalización.
    }

    if (r[0] == 0.0F) //to avoid log() and divide zero 
      r[0] = 1e-10; 
  }

  void PitchAnalyzer::set_window(Window win_type) {
    if (frameLen == 0)
      return;

    window.resize(frameLen);

    switch (win_type) {
    case HAMMING:
      /// \TODO Implement the Hamming window
      /// \HECHO
      for (size_t n = 0; n < frameLen; ++n) {
        float window_value = 0.54f - 0.46f * std::cos(2.0f * M_PI * n / (frameLen - 1));
        window[n] = window_value;
      }
      break;
    case RECT:
    default:
      window.assign(frameLen, 1);
    }
  }

  void PitchAnalyzer::set_f0_range(float min_F0, float max_F0) {
    npitch_min = (unsigned int) samplingFreq/max_F0;
    if (npitch_min < 2)
      npitch_min = 2;  // samplingFreq/2

    npitch_max = 1 + (unsigned int) samplingFreq/min_F0;

    //frameLen should include at least 2*T0
    if (npitch_max > frameLen/2)
      npitch_max = frameLen/2;
  }

  bool PitchAnalyzer::unvoiced(float pot, float r1norm, float rmaxnorm, float zcr) const {
    /// \TODO Implement a rule to decide whether the sound is voiced or not.
    /// * You can use the standard features (pot, r1norm, rmaxnorm),
    ///   or compute and use other ones.
    /// \HECHO criterio de decisión voiced o unvoiced
    const float pot_threshold = -40.0f;
    const float r1norm_threshold = 0.2f;
    const float rmaxnorm_threshold = 0.4f;
    const float zcr_threshold = 0.15f;

    if (pot < pot_threshold || r1norm < r1norm_threshold || rmaxnorm < rmaxnorm_threshold || zcr > zcr_threshold) {
      return true;//unvoiced
    } else {
      return false;//voiced
    }
    
  }

  float PitchAnalyzer::compute_pitch(vector<float> & x) const {
    if (x.size() != frameLen)
      return -1.0F;

    //Window input frame
    for (unsigned int i=0; i<x.size(); ++i)
      x[i] *= window[i];

    vector<float> r(npitch_max);

    //Compute correlation
    autocorrelation(x, r);

    //vector<float>::const_iterator iR = r.begin(), iRMax = iR;
    unsigned int lag = npitch_min;
    float max_corr = r[npitch_min];//r[0] no ens interessa.

    for (unsigned int i = npitch_min; i < npitch_max; ++i) {
      if (r[i] > max_corr) {
        max_corr = r[i];
        lag = i;
      }
    }

    /// \TODO 
	/// Find the lag of the maximum value of the autocorrelation away from the origin.<br>
	/// Choices to set the minimum value of the lag are:
	///    - The first negative value of the autocorrelation.
	///    - The lag corresponding to the maximum value of the pitch.
    ///	   .
	/// In either case, the lag should not exceed that of the minimum value of the pitch.
  /// \HECHO hemos hecho la búsqueda del primer máximo secundario, excluyendo r[0]

    //unsigned int lag = iRMax - r.begin();

    float pot = 10 * log10(r[0]);
    float zcr = 0;
    for (size_t i = 1; i < x.size(); ++i) {
      if ((x[i - 1] >= 0 && x[i] < 0) || (x[i - 1] < 0 && x[i] >= 0)) {
        zcr += 1;
      }
    }
    zcr /= static_cast<float>(x.size());

    //You can print these (and other) features, look at them using wavesurfer
    //Based on that, implement a rule for unvoiced
    //change to #if 1 and compile
#if 0
    if (r[0] > 0.0F)
      cout << pot << '\t' << r[1]/r[0] << '\t' << r[lag]/r[0] << '\t' << lag << endl;
#endif
    
    if (unvoiced(pot, r[1]/r[0], r[lag]/r[0], zcr) || lag == 0)
      return 0;
    else
      return (float) samplingFreq/(float) lag;
  }

}