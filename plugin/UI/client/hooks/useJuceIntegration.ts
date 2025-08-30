import { useState, useEffect, useCallback } from "react";

// Define the structure of a parameter
interface Parameter {
  id: string;
  value: number;
}

// Define the structure of the message from the JUCE backend
interface JuceMessage {
  type: string;
  payload: Parameter;
}

// Custom hook for JUCE integration
export function useJuceIntegration() {
  const [parameters, setParameters] = useState<Record<string, number>>({});

  const handleJuceMessage = useCallback((event: MessageEvent) => {
    if (
      typeof event.data === "object" &&
      event.data.type === "parameterChanged"
    ) {
      const { id, value } = (event.data as JuceMessage).payload;
      setParameters((prevParams) => ({
        ...prevParams,
        [id]: value,
      }));
    }
  }, []);

  useEffect(() => {
    window.addEventListener("message", handleJuceMessage);
    return () => {
      window.removeEventListener("message", handleJuceMessage);
    };
  }, [handleJuceMessage]);

  const setParameterValue = useCallback((id: string, value: number) => {
    console.log(`[TS] About to set parameter ${id} to ${value}`);

    // Usa il sistema di navigazione URL per comunicare con JUCE
    try {
      // Crea un URL speciale che il C++ può intercettare
      const paramUrl = `juce://setParameter?id=${encodeURIComponent(
        id
      )}&value=${value}`;

      console.log(`[TS] Creating navigation URL: ${paramUrl}`);

      // Crea un iframe temporaneo per innescare la navigazione
      const iframe = document.createElement("iframe");
      iframe.style.display = "none";
      iframe.src = paramUrl;
      document.body.appendChild(iframe);

      console.log(`[TS] Iframe created and added to DOM`);

      // Rimuovi l'iframe dopo un breve delay
      setTimeout(() => {
        if (document.body.contains(iframe)) {
          document.body.removeChild(iframe);
          console.log(`[TS] Iframe removed from DOM`);
        }
      }, 100);

      console.log(`Parameter ${id} set to ${value}`);
    } catch (error) {
      console.error("Error setting parameter:", error);
    }

    // Aggiornamento ottimistico dell'UI
    setParameters((prevParams) => ({
      ...prevParams,
      [id]: value,
    }));
  }, []);

  return { parameters, setParameterValue };
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
