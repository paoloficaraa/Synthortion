// Dichiarazioni di tipo per JUCE Web Integration
declare module "juce/index.js" {
  export interface SliderState {
    getNormalisedValue(): number;
    setNormalisedValue(value: number): void;
    valueChangedEvent: {
      addListener(callback: () => void): void;
      removeListener(callback: () => void): void;
    };
  }

  export function getSliderState(parameterId: string): SliderState;
  export function beginParameterChangeGesture(parameterId: string): void;
  export function endParameterChangeGesture(parameterId: string): void;
}
