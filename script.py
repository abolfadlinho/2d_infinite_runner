import subprocess
import wave
import pyaudio

def play_sound():
    # Your sound playing code here
    # Example:
    wf = wave.open("Sounds/background.wav", 'rb')
    p = pyaudio.PyAudio()
    stream = p.open(format=p.get_format_from_width(wf.getsampwidth()),
                    channels=wf.getnchannels(),
                    rate=wf.getframerate(),
                    output=True)
    data = wf.readframes(1024)
    while data:
        stream.write(data)
        data = wf.readframes(1024)
    stream.stop_stream()
    stream.close()
    p.terminate()

if __name__ == "__main__":
    play_sound()
