import React, { useState, useRef, useCallback } from "react";

interface KnobProps {
  value: number;
  onChange: (value: number) => void;
  min?: number;
  max?: number;
  size?: "small" | "medium" | "large";
  label?: string;
  className?: string;
}

export default function Knob({
  value,
  onChange,
  min = 0,
  max = 100,
  size = "medium",
  label,
  className,
}: KnobProps) {
  const [isDragging, setIsDragging] = useState(false);

  const sizeClasses = {
    tiny: "w-10 h-10",
    small: "w-12 h-12",
    medium: "w-16 h-16",
    large: "w-24 h-24",
  };

  const normalizedValue = (value - min) / (max - min);
  const rotation = normalizedValue * 300 - 150; // -150° to +150°

  const handleMouseDown = useCallback((e: React.MouseEvent) => {
    setIsDragging(true);
    e.preventDefault();
  }, []);

  const handleMouseMove = useCallback(
    (e: MouseEvent) => {
      if (!isDragging) return;

      const deltaY = e.movementY;
      const sensitivity = 0.5;
      const change = -deltaY * sensitivity;
      const newValue = Math.min(max, Math.max(min, value + change));
      onChange(newValue);
    },
    [isDragging, value, min, max, onChange]
  );

  const handleMouseUp = useCallback(() => {
    setIsDragging(false);
  }, []);

  React.useEffect(() => {
    if (isDragging) {
      document.addEventListener("mousemove", handleMouseMove);
      document.addEventListener("mouseup", handleMouseUp);
      document.body.style.cursor = "grabbing";
      return () => {
        document.removeEventListener("mousemove", handleMouseMove);
        document.removeEventListener("mouseup", handleMouseUp);
        document.body.style.cursor = "";
      };
    }
  }, [isDragging, handleMouseMove, handleMouseUp]);

  return (
    <div className={`flex flex-col items-center ${className}`}>
      <div
        className={`
          ${sizeClasses[size]} 
          relative cursor-pointer select-none
          bg-gradient-to-b from-gray-600 to-gray-800 
          rounded-full shadow-lg border-2 border-gray-500
          transform transition-transform
          ${isDragging ? "scale-105" : "hover:scale-105"}
        `}
        onMouseDown={handleMouseDown}
        style={{
          background: `conic-gradient(from 0deg, 
            #4a5568 0%, 
            #2d3748 25%, 
            #1a202c 50%, 
            #2d3748 75%, 
            #4a5568 100%)`,
        }}
      >
        {/* Inner shadow ring */}
        <div className="absolute inset-1 rounded-full shadow-inner bg-gradient-to-br from-gray-700 to-gray-800" />

        {/* Center cap */}
        <div className="absolute inset-3 rounded-full bg-gradient-to-br from-gray-600 to-gray-800 shadow-inner border border-gray-600" />

        {/* Indicator */}
        <div
          className="absolute inset-0 flex items-start justify-center pt-1"
          style={{ transform: `rotate(${rotation}deg)` }}
        >
          <div className="w-0.5 h-3 bg-orange-400 rounded-full shadow-lg" />
        </div>

        {/* Center dot */}
        <div className="absolute inset-0 flex items-center justify-center">
          <div className="w-1 h-1 bg-gray-500 rounded-full" />
        </div>
      </div>

      {label && (
        <div className="text-center mt-2 text-xs text-gray-300 font-bold">
          {label}
        </div>
      )}
    </div>
  );
}
