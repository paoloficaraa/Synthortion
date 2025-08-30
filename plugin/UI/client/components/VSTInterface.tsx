// Import della libreria JUCE
import * as Juce from "juce/index.js";
import Knob from "./ui/Knob";
import VSTDisplay from "./ui/VSTDisplay";
import LevelMeter from "./ui/LevelMeter";
import EQSection from "./ui/EQSection";
import Select from "./ui/Select";
import { useEffect, useState } from "react";

export default function VSTInterface() {
  // Stati locali per i parametri
  const [parameters, setParameters] = useState({
    DRIVE: 0.3,
    MIX: 1.0,
    SATURATION_TYPE: 0, // 0 = SMOOTH, 1 = TUBE, 2 = TAPE
    INPUT_GAIN: 0,
    OUTPUT_GAIN: 0,
    DELAY: 0,
    CHORUS: 0,
  });

  // Stati JUCE per i parametri
  const [driveState, setDriveState] = useState<any>(null);
  const [mixState, setMixState] = useState<any>(null);
  const [saturationTypeState, setSaturationTypeState] = useState<any>(null);

  useEffect(() => {
    console.log("JUCE frontend library successfully imported");

    // Inizializza gli stati dei parametri JUCE
    try {
      const drive = Juce.getSliderState("DRIVE");
      const mix = Juce.getSliderState("MIX");
      const saturationType = Juce.getSliderState("SATURATION_TYPE");

      setDriveState(drive);
      setMixState(mix);
      setSaturationTypeState(saturationType);

      // Listener per i cambiamenti dei parametri dal backend
      drive.valueChangedEvent.addListener(() => {
        const newValue = drive.getNormalisedValue();
        setParameters((prev) => ({ ...prev, DRIVE: newValue }));
        console.log(`[JUCE] DRIVE parameter changed to: ${newValue}`);
      });

      mix.valueChangedEvent.addListener(() => {
        const newValue = mix.getNormalisedValue();
        setParameters((prev) => ({ ...prev, MIX: newValue }));
        console.log(`[JUCE] MIX parameter changed to: ${newValue}`);
      });

      saturationType.valueChangedEvent.addListener(() => {
        const newValue = saturationType.getNormalisedValue() * 2; // 0-2 range
        setParameters((prev) => ({
          ...prev,
          SATURATION_TYPE: Math.round(newValue),
        }));
        console.log(
          `[JUCE] SATURATION_TYPE parameter changed to: ${Math.round(newValue)}`
        );
      });

      // Imposta i valori iniziali
      setParameters((prev) => ({
        ...prev,
        DRIVE: drive.getNormalisedValue(),
        MIX: mix.getNormalisedValue(),
        SATURATION_TYPE: Math.round(saturationType.getNormalisedValue() * 2),
      }));
    } catch (error) {
      console.error("Error initializing JUCE states:", error);
    }
  }, []);

  const setParameterValue = (paramId: string, value: number) => {
    if (paramId === "DRIVE" && driveState) {
      driveState.setNormalisedValue(value);
      console.log(`[JUCE] Setting DRIVE to: ${value}`);
    } else if (paramId === "MIX" && mixState) {
      mixState.setNormalisedValue(value);
      console.log(`[JUCE] Setting MIX to: ${value}`);
    } else if (paramId === "SATURATION_TYPE" && saturationTypeState) {
      saturationTypeState.setNormalisedValue(value / 2); // Convert 0-2 to 0-1
      console.log(`[JUCE] Setting SATURATION_TYPE to: ${value}`);
    }

    // Aggiorna lo stato locale anche per gli altri parametri (non JUCE)
    setParameters((prev) => ({ ...prev, [paramId]: value }));
  };

  // Get normalized parameter values (0-1) or default to 0
  const drive = parameters.DRIVE ?? 0;
  const mix = parameters.MIX ?? 1.0;
  const saturationType = parameters.SATURATION_TYPE ?? 0;
  const inputGain = parameters.INPUT_GAIN ?? 0;
  const outputGain = parameters.OUTPUT_GAIN ?? 0;
  const delay = parameters.DELAY ?? 0;
  const chorus = parameters.CHORUS ?? 0;

  // Saturation type options
  const saturationOptions = [
    { value: 0, label: "Smooth" },
    { value: 1, label: "Tube" },
    { value: 2, label: "Tape" },
  ];

  // Dummy values for now, you can connect these later
  const isConnected = true;
  const inputLevel = 0.5;
  const outputLevel = 0.5;
  const postDistortionFFT = new Float32Array(128).fill(0);

  return (
    <div
      className="relative w-full bg-gradient-to-br from-gray-800 via-gray-900 to-black shadow-2xl border border-gray-700"
      style={{ width: "1024px", height: "650px" }}
    >
      {/* Main container with metallic frame effect */}
      <div className="absolute inset-2 bg-gradient-to-br from-gray-700 via-gray-800 to-gray-900 rounded-2xl shadow-inner">
        {/* Header */}
        <div className="flex items-center justify-between px-6 py-3 bg-gradient-to-r from-gray-600 to-gray-700 rounded-t-2xl border-b border-gray-600">
          <div className="flex items-center gap-2">
            <div
              className={`w-3 h-3 rounded-full shadow-inner ${
                isConnected ? "bg-green-500" : "bg-red-500"
              }`}
            ></div>
            <span className="text-white font-bold text-xl tracking-wider">
              Synthortion
            </span>
          </div>
          <div className="flex gap-1">
            {[...Array(12)].map((_, i) => (
              <div
                key={i}
                className="w-1 h-4 bg-gray-500 rounded-sm opacity-60"
              ></div>
            ))}
          </div>
        </div>

        <div
          className="flex h-full p-4 gap-4"
          style={{ height: "calc(100% - 80px)" }}
        >
          {/* Left side - Input and controls */}
          <div className="flex flex-col items-center w-1/4 min-w-0">
            {/* Input level */}
            <div className="flex flex-col items-center mb-6">
              <span className="text-gray-300 text-xs mb-2 font-bold">
                INPUT GAIN
              </span>
              <LevelMeter
                value={inputLevel * inputGain}
                orientation="vertical"
                color="orange"
              />
              <Knob
                value={inputGain * 100}
                onChange={(value) =>
                  setParameterValue("INPUT_GAIN", value / 100)
                }
                size="small"
                label=""
                className="mt-2"
              />
            </div>

            {/* Drive Knob */}
            <div className="flex flex-col items-center justify-center mb-6">
              <Knob
                value={drive * 100}
                onChange={(value) => setParameterValue("DRIVE", value / 100)}
                size="large"
                label=""
              />
              <span className="text-gray-300 text-xs mt-2 font-bold">
                DRIVE
              </span>
            </div>

            {/* Bottom knobs - Delay and Chorus */}
            <div className="flex gap-4 mt-auto pb-4">
              <div className="flex flex-col items-center">
                <Knob
                  value={delay * 100}
                  onChange={(value) => setParameterValue("DELAY", value / 100)}
                  size="medium"
                  label=""
                />
                <span className="text-gray-300 text-xs mt-2 font-bold">
                  Delay
                </span>
              </div>
              <div className="flex flex-col items-center">
                <Knob
                  value={chorus * 100}
                  onChange={(value) => setParameterValue("CHORUS", value / 100)}
                  size="medium"
                  label=""
                />
                <span className="text-gray-300 text-xs mt-2 font-bold">
                  Chorus
                </span>
              </div>
            </div>
          </div>

          {/* Center - Display and EQ */}
          <div className="flex-1 flex flex-col px-2">
            {/* Main display */}
            <div className="mb-3">
              <div
                className="bg-black border-2 border-gray-600 rounded-lg p-3"
                style={{ height: "300px" }}
              >
                <VSTDisplay audioData={postDistortionFFT || undefined} />
              </div>
            </div>

            {/* EQ Section */}
            <div style={{ height: "500px" }}>
              <EQSection />
            </div>
          </div>

          {/* Right side - Output and controls */}
          <div className="flex flex-col items-center w-1/4 min-w-0">
            {/* Output level */}
            <div className="flex flex-col items-center mb-6">
              <span className="text-gray-300 text-xs mb-2 font-bold">
                OUTPUT GAIN
              </span>
              <LevelMeter
                value={outputLevel * outputGain}
                orientation="vertical"
                color="orange"
              />
              <Knob
                value={outputGain * 100}
                onChange={(value) =>
                  setParameterValue("OUTPUT_GAIN", value / 100)
                }
                size="small"
                label=""
                className="mt-2"
              />
            </div>

            {/* Saturation Type Selector */}
            <div className="flex flex-col items-center mb-6">
              <span className="text-gray-300 text-xs mb-2 font-bold block text-center">
                TYPE
              </span>
              <Select
                value={saturationType}
                onChange={(value) =>
                  setParameterValue("SATURATION_TYPE", value)
                }
                options={saturationOptions}
                className="w-full"
              />
            </div>

            {/* Dry/Wet */}
            <div className="flex flex-col items-center justify-center mt-auto">
              <Knob
                value={mix * 100}
                onChange={(value) => setParameterValue("MIX", value / 100)}
                size="medium"
                label=""
              />
              <span className="text-gray-300 text-xs mt-2 font-bold">
                DRY/WET
              </span>
            </div>
          </div>
        </div>
      </div>

      {/* Corner screws effect */}
      <div className="absolute top-4 left-4 w-3 h-3 bg-gray-600 rounded-full shadow-inner border border-gray-500"></div>
      <div className="absolute top-4 right-4 w-3 h-3 bg-gray-600 rounded-full shadow-inner border border-gray-500"></div>
      <div className="absolute bottom-4 left-4 w-3 h-3 bg-gray-600 rounded-full shadow-inner border border-gray-500"></div>
      <div className="absolute bottom-4 right-4 w-3 h-3 bg-gray-600 rounded-full shadow-inner border border-gray-500"></div>
    </div>
  );
}
