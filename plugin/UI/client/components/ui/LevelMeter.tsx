import React from "react";

interface LevelMeterProps {
  value: number;
  orientation?: "horizontal" | "vertical";
  segments?: number;
  color?: "blue" | "orange" | "green" | "red";
  label?: string;
  className?: string;
}

export default function LevelMeter({
  value,
  orientation = "vertical",
  segments = 16,
  color = "green",
  label,
  className = "",
}: LevelMeterProps) {
  const segmentValue = 100 / segments;
  const colorClasses = {
    blue: "bg-blue-500",
    orange: "bg-orange-500",
    green: "bg-green-500",
    red: "bg-red-500",
  };

  const activeSegments = Math.floor(value / segmentValue);

  if (orientation === "vertical") {
    return (
      <div
        className={`flex flex-col-reverse gap-0.5 p-2 bg-black rounded border border-gray-600 shadow-inner w-6 h-32 ${className}`}
      >
        {[...Array(segments)].map((_, i) => (
          <div
            key={i}
            className={`w-full h-2 rounded-sm transition-all duration-150 ${
              i < activeSegments ? colorClasses[color] : "bg-gray-800"
            }`}
          />
        ))}
        {label && (
          <span className="text-xs text-gray-300 mt-1 text-center font-semibold">
            {label}
          </span>
        )}
      </div>
    );
  }

  return (
    <div
      className={`flex gap-0.5 p-2 bg-black rounded border border-gray-600 shadow-inner h-6 w-32 ${className}`}
    >
      {[...Array(segments)].map((_, i) => (
        <div
          key={i}
          className={`w-2 h-full rounded-sm transition-all duration-150 ${
            i < activeSegments ? colorClasses[color] : "bg-gray-800"
          }`}
        />
      ))}
      {label && (
        <span className="text-xs text-gray-300 ml-2 font-semibold">
          {label}
        </span>
      )}
    </div>
  );
}
