import { useState, useEffect, useCallback } from "react";

// Hook for integrating with JUCE WebBrowserComponent
export function useJuceIntegration() {
  const [isConnected, setIsConnected] = useState(false);
  const [parameters, setParameters] = useState<{ [key: string]: number }>({});
  const [postDistortionFFT, setPostDistortionFFT] =
    useState<Float32Array | null>(null);
  const [inputLevel, setInputLevel] = useState(0);
  const [outputLevel, setOutputLevel] = useState(0);

  // Send parameter change to JUCE
  const setParameter = useCallback((paramId: string, value: number) => {
    if (window.juce && window.juce.setParameter) {
      window.juce.setParameter(paramId, value);
    }
    setParameters((prev) => ({
      ...prev,
      [paramId]: value,
    }));
  }, []);

  // Listen for parameter updates from JUCE
  useEffect(() => {
    const handleParameterUpdate = (event: CustomEvent) => {
      const { paramId, value } = event.detail;
      setParameters((prev) => ({
        ...prev,
        [paramId]: value,
      }));
    };

    // Handle post-distortion FFT data from JUCE
    const handleFFTUpdate = (event: CustomEvent) => {
      const { fftData } = event.detail;
      if (fftData instanceof Float32Array) {
        setPostDistortionFFT(fftData);
      }
    };

    // Handle level meters update
    const handleLevelsUpdate = (event: CustomEvent) => {
      const { inputLevel, outputLevel } = event.detail;
      setInputLevel(inputLevel || 0);
      setOutputLevel(outputLevel || 0);
    };

    // Check if JUCE bridge is available
    const checkJuceConnection = () => {
      if (window.juce) {
        setIsConnected(true);
        // Setup event listeners
        window.addEventListener(
          "juceParameterUpdate",
          handleParameterUpdate as EventListener
        );
        window.addEventListener(
          "jucePostDistortionFFT",
          handleFFTUpdate as EventListener
        );
        window.addEventListener(
          "juceLevelsUpdate",
          handleLevelsUpdate as EventListener
        );
      } else {
        setIsConnected(false);
        // Reset to null/zero values when disconnected
        setPostDistortionFFT(null);
        setInputLevel(0);
        setOutputLevel(0);
      }
    };

    checkJuceConnection();

    // Recheck connection periodically
    const connectionCheck = setInterval(checkJuceConnection, 1000);

    return () => {
      clearInterval(connectionCheck);
      window.removeEventListener(
        "juceParameterUpdate",
        handleParameterUpdate as EventListener
      );
      window.removeEventListener(
        "jucePostDistortionFFT",
        handleFFTUpdate as EventListener
      );
      window.removeEventListener(
        "juceLevelsUpdate",
        handleLevelsUpdate as EventListener
      );
    };
  }, []);

  return {
    isConnected,
    parameters,
    setParameter,
    postDistortionFFT,
    inputLevel,
    outputLevel,
  };
}

// Extend window interface for JUCE bridge
declare global {
  interface Window {
    juce?: {
      setParameter: (paramId: string, value: number) => void;
      getParameter: (paramId: string) => number;
      // Additional methods that might be available from JUCE
      getPostDistortionFFT?: () => Float32Array;
      getLevels?: () => { input: number; output: number };
    };
  }
}
