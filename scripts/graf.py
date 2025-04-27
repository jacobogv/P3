import numpy as np
import matplotlib.pyplot as plt
from scipy.io import wavfile
from scipy.signal import correlate

# Función para calcular la autocorrelación y encontrar el primer máximo secundario
def calcular_autocorrelacion(signal, fs):
    # Normalizar la señal
    signal = signal - np.mean(signal)  # Centrar la señal en torno a 0
    
    # Calcular la autocorrelación
    autocorr = correlate(signal, signal, mode='full')
    autocorr = autocorr[autocorr.size // 2:]  # Solo la mitad positiva
    lags = np.arange(0, len(autocorr)) / fs  # Lags en segundos
    
    # Detectar el primer máximo secundario
    peaks = np.where(np.diff(np.sign(np.diff(autocorr))) == -2)[0]  # Detectar picos
    if len(peaks) > 1:
        first_secondary_peak = peaks[1]  # El primer máximo secundario
    else:
        first_secondary_peak = None  # Si no hay suficientes picos secundarios
    
    return autocorr, lags, first_secondary_peak

# Cargar la señal WAV
filename = 'graficar.wav'  # Cambia esto por el nombre de tu archivo WAV
fs, signal = wavfile.read(filename)

# Si la señal tiene más de un canal, tomar solo el primero (mono)
if len(signal.shape) > 1:
    signal = signal[:, 0]

# Definir la duración del segmento que queremos analizar (30 ms)
duration = 0.03  # 30 ms
n_samples = int(fs * duration)

# Tomar el primer segmento de 30 ms
signal_segment = signal[:n_samples]

# Calcular la autocorrelación y el primer máximo secundario
autocorr, lags, first_secondary_peak = calcular_autocorrelacion(signal_segment, fs)

# Graficar la señal temporal y su autocorrelación
fig, axs = plt.subplots(2, 1, figsize=(10, 8))

# Subgráfico 1: Señal temporal
time = np.arange(n_samples) / fs  # Tiempo en segundos
axs[0].plot(time, signal_segment, label='Señal temporal', color='blue')
axs[0].set_title('Señal Temporal del Fonema Sonoro (30 ms)')
axs[0].set_xlabel('Tiempo [s]')
axs[0].set_ylabel('Amplitud')
axs[0].grid(True)

# Subgráfico 2: Autocorrelación
axs[1].plot(lags, autocorr, label='Autocorrelación', color='green')
if first_secondary_peak is not None:
    axs[1].plot(lags[first_secondary_peak], autocorr[first_secondary_peak], 'ro', label='Primer máximo secundario')
axs[1].set_title('Autocorrelación de la Señal')
axs[1].set_xlabel('Desplazamiento [s]')
axs[1].set_ylabel('Autocorrelación')
axs[1].grid(True)
axs[1].legend()

plt.tight_layout()
plt.show()
