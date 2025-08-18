import { useState } from "react";
import { useJuceIntegration } from "../hooks/useJuceIntegration";
import Knob from "./ui/Knob";
import VSTDisplay from "./ui/VSTDisplay";
import LevelMeter from "./ui/LevelMeter";
import EQSection from "./ui/EQSection";

// Plugin parameter interface for type safety
interface PluginParameters {
  color: number;
  delay: number;
  chorus: number;
  dryWet: number;
  inputGain: number;
  outputGain: number;
}

export default function VSTInterface() {
  // Plugin parameter states with default values
  const [parameters, setParameters] = useState<PluginParameters>({
    color: 0,
    delay: 0,
    chorus: 0,
    dryWet: 100,
    inputGain: 0,
    outputGain: 0,
  });

  // JUCE integration hook
  const {
    isConnected,
    setParameter,
    postDistortionFFT,
    inputLevel,
    outputLevel,
  } = useJuceIntegration();

  // Generic parameter change handler
  const handleParameterChange = (
    paramName: keyof PluginParameters,
    value: number
  ) => {
    setParameters((prev) => ({ ...prev, [paramName]: value }));
    setParameter(paramName, value / 100); // Normalize to 0-1 for JUCE
  };

  return (
    <div
      className="relative w-full max-w-5xl bg-gradient-to-br from-gray-800 via-gray-900 to-black rounded-3xl shadow-2xl border border-gray-700"
      style={{ minHeight: "650px", height: "auto" }}
    >
      {/* Main container with metallic frame effect */}
      <div className="absolute inset-2 bg-gradient-to-br from-gray-700 via-gray-800 to-gray-900 rounded-2xl shadow-inner">
        {/* Header */}
        <div className="flex items-center justify-between p-4 bg-gradient-to-r from-gray-600 to-gray-700 rounded-t-2xl border-b border-gray-600">
          <div className="flex items-center gap-2">
            <div
              className={`w-3 h-3 rounded-full shadow-inner ${
                isConnected ? "bg-green-500" : "bg-red-500"
              }`}
            ></div>
            <span className="text-white font-bold text-2xl tracking-wider">
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

        <div className="flex min-h-[550px] p-4">
          {/* Left side - Input and Color knob */}
          <div className="flex flex-col items-center w-1/4 space-y-6">
            {/* Input level */}
            <div className="flex flex-col items-center">
              <span className="text-gray-300 text-xs mb-2 font-bold">
                INPUT GAIN
              </span>
              <LevelMeter
                value={inputLevel * (parameters.inputGain / 100)}
                orientation="vertical"
                color="orange"
              />
              <Knob
                value={parameters.inputGain}
                onChange={(value) => handleParameterChange("inputGain", value)}
                size="small"
                label=""
                className="mt-2"
              />
            </div>

            {/* Color knob */}
            <div className="flex flex-col items-center flex-1 justify-center">
              <div className="bg-gradient-to-b from-orange-600 to-orange-800 px-3 py-1 rounded-lg border border-orange-500 mb-4">
                <span className="text-white text-sm font-bold">COLOR</span>
              </div>
              <Knob
                value={parameters.color}
                onChange={(value) => handleParameterChange("color", value)}
                size="large"
                label=""
              />
            </div>

            {/* Bottom knobs - Delay and Chorus */}
            <div className="flex gap-6 pb-4">
              <div className="flex flex-col items-center">
                <Knob
                  value={parameters.delay}
                  onChange={(value) => handleParameterChange("delay", value)}
                  size="medium"
                  label=""
                />
                <span className="text-gray-300 text-xs mt-2 font-bold">
                  Delay
                </span>
              </div>
              <div className="flex flex-col items-center">
                <Knob
                  value={parameters.chorus}
                  onChange={(value) => handleParameterChange("chorus", value)}
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
          <div className="flex-1 flex flex-col px-6">
            {/* Main display */}
            <div className="flex-1 mb-6">
              <div className="bg-black border-2 border-gray-600 rounded-lg p-3 h-64">
                <VSTDisplay audioData={postDistortionFFT || undefined} />
              </div>
            </div>

            {/* EQ Section */}
            <div className="min-h-[200px]">
              <EQSection />
            </div>
          </div>

          {/* Right side - Output and Dry/Wet */}
          <div className="flex flex-col items-center w-1/4 space-y-6">
            {/* Output level */}
            <div className="flex flex-col items-center">
              <span className="text-gray-300 text-xs mb-2 font-bold">
                OUTPUT GAIN
              </span>
              <LevelMeter
                value={outputLevel * (parameters.outputGain / 100)}
                orientation="vertical"
                color="orange"
              />
              <Knob
                value={parameters.outputGain}
                onChange={(value) => handleParameterChange("outputGain", value)}
                size="small"
                label=""
                className="mt-2"
              />
            </div>

            {/* Dry/Wet */}
            <div className="flex flex-col items-center flex-1 justify-center pb-16">
              <Knob
                value={parameters.dryWet}
                onChange={(value) => handleParameterChange("dryWet", value)}
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
