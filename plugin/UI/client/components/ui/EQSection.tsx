import { useState } from "react";
import Knob from "./Knob";

export default function EQSection() {
  const [lowCut, setLowCut] = useState(0);
  const [lowMid, setLowMid] = useState(40);
  const [highMid, setHighMid] = useState(60);
  const [highCut, setHighCut] = useState(100);
  const [q1, setQ1] = useState(50);
  const [q2, setQ2] = useState(50);
  const [q3, setQ3] = useState(50);
  const [q4, setQ4] = useState(50);
  const [lowMidGain, setLowMidGain] = useState(50);
  const [highMidGain, setHighMidGain] = useState(50);

  // Functions to convert knob values (0-100) to frequency ranges
  const getLowCutFreq = (value: number) => {
    // 20Hz to 250Hz
    const freq = 20 + (value / 100) * (250 - 20);
    return `${Math.round(freq)}`;
  };

  const getLowMidFreq = (value: number) => {
    // 250Hz to 500Hz
    const freq = 250 + (value / 100) * (500 - 250);
    return `${Math.round(freq)}`;
  };

  const getHighMidFreq = (value: number) => {
    // 2kHz to 5kHz
    const freq = 2000 + (value / 100) * (5000 - 2000);
    return `${(freq / 1000).toFixed(1)}k`;
  };

  const getHighCutFreq = (value: number) => {
    // 5kHz to 20kHz
    const freq = 5000 + (value / 100) * (20000 - 5000);
    return `${(freq / 1000).toFixed(1)}k`;
  };

  return (
    <div className="bg-gradient-to-b from-gray-700 to-gray-800 rounded-lg border border-gray-600 p-2">
      {/* Header */}
      <div className="flex items-center justify-between mb-2">
        <div className="bg-orange-500 px-2 py-1 rounded text-xs font-bold text-black">
          EQ / FILTER
        </div>
      </div>

      {/* EQ Controls */}
      <div className="grid grid-cols-4 gap-1">
        {/* Low Cut */}
        <div className="flex flex-col items-center">
          <div className="text-xs text-gray-300 mb-1 text-center">
            <div className="font-bold">Low Cut (Hz)</div>
            <div className="text-orange-400">{getLowCutFreq(lowCut)}</div>
          </div>
          <Knob value={lowCut} onChange={setLowCut} size="tiny" label="" />
          <div className="text-xs text-gray-400 mt-1">Q (Hz)</div>
          <Knob
            value={q1}
            onChange={setQ1}
            size="tiny"
            label=""
            className="mt-1"
          />
        </div>

        {/* Low Mid */}
        <div className="flex flex-col items-center">
          <div className="text-xs text-gray-300 mb-1 text-center">
            <div className="font-bold">Low Mid (Hz)</div>
            <div className="text-orange-400">{getLowMidFreq(lowMid)}</div>
          </div>
          <Knob value={lowMid} onChange={setLowMid} size="tiny" label="" />
          <div className="flex gap-2 mt-1">
            <div className="flex flex-col items-center">
              <div className="text-xs text-gray-400">Gain</div>
              <Knob
                value={lowMidGain}
                onChange={setLowMidGain}
                size="tiny"
                label=""
              />
            </div>
            <div className="flex flex-col items-center">
              <div className="text-xs text-gray-400">Q</div>
              <Knob value={q2} onChange={setQ2} size="tiny" label="" />
            </div>
          </div>
        </div>

        {/* High Mid */}
        <div className="flex flex-col items-center">
          <div className="text-xs text-gray-300 mb-1 text-center">
            <div className="font-bold">High Mid (Hz)</div>
            <div className="text-orange-400">{getHighMidFreq(highMid)}</div>
          </div>
          <Knob value={highMid} onChange={setHighMid} size="tiny" label="" />
          <div className="flex gap-2 mt-1">
            <div className="flex flex-col items-center">
              <div className="text-xs text-gray-400">Gain</div>
              <Knob
                value={highMidGain}
                onChange={setHighMidGain}
                size="tiny"
                label=""
              />
            </div>
            <div className="flex flex-col items-center">
              <div className="text-xs text-gray-400">Q</div>
              <Knob value={q3} onChange={setQ3} size="tiny" label="" />
            </div>
          </div>
        </div>

        {/* High Cut */}
        <div className="flex flex-col items-center">
          <div className="text-xs text-gray-300 mb-1 text-center">
            <div className="font-bold">High Cut (Hz)</div>
            <div className="text-orange-400">{getHighCutFreq(highCut)}</div>
          </div>
          <Knob value={highCut} onChange={setHighCut} size="tiny" label="" />
          <div className="text-xs text-gray-400 mt-1">Q (Hz)</div>
          <Knob
            value={q4}
            onChange={setQ4}
            size="tiny"
            label=""
            className="mt-1"
          />
        </div>
      </div>
    </div>
  );
}
