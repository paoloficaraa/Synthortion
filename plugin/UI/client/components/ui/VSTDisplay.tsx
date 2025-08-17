import { useEffect, useRef } from "react";

interface VSTDisplayProps {
  audioData?: Float32Array;
}

export default function VSTDisplay({ audioData }: VSTDisplayProps) {
  const canvasRef = useRef<HTMLCanvasElement>(null);

  useEffect(() => {
    const canvas = canvasRef.current;
    if (!canvas) return;

    const ctx = canvas.getContext("2d");
    if (!ctx) return;

    // Set canvas size for high DPI displays
    const rect = canvas.getBoundingClientRect();
    const displayWidth = rect.width;
    const displayHeight = rect.height;

    canvas.width = displayWidth;
    canvas.height = displayHeight;

    // Clear canvas with dark background
    ctx.fillStyle = "#1a1a1a";
    ctx.fillRect(0, 0, displayWidth, displayHeight);

    // If no audio data, just show static grid
    if (!audioData) {
      drawStaticGrid(ctx, displayWidth, displayHeight);
      return;
    }

    // If we have audio data, draw the frequency visualization
    drawFrequencyBars(ctx, audioData, displayWidth, displayHeight);
  }, [audioData]);

  const drawStaticGrid = (
    ctx: CanvasRenderingContext2D,
    w: number,
    h: number,
  ) => {
    ctx.strokeStyle = "rgba(255, 255, 255, 0.05)";
    ctx.lineWidth = 0.5;

    // Horizontal frequency lines
    for (let i = 1; i < 8; i++) {
      const y = (h / 8) * i;
      ctx.beginPath();
      ctx.moveTo(0, y);
      ctx.lineTo(w, y);
      ctx.stroke();
    }

    // Vertical time lines
    for (let i = 1; i < 16; i++) {
      const x = (w / 16) * i;
      ctx.beginPath();
      ctx.moveTo(x, 0);
      ctx.lineTo(x, h);
      ctx.stroke();
    }

    // Add center text indicating no signal
    ctx.fillStyle = "rgba(255, 255, 255, 0.2)";
    ctx.font = "12px monospace";
    ctx.textAlign = "center";
    ctx.fillText("NO SIGNAL", w / 2, h / 2);
  };

  const drawFrequencyBars = (
    ctx: CanvasRenderingContext2D,
    fftData: Float32Array,
    w: number,
    h: number,
  ) => {
    // Draw frequency bars
    const barWidth = w / fftData.length;

    for (let i = 0; i < fftData.length; i++) {
      const magnitude = fftData[i];
      const barHeight = magnitude * h * 0.8;

      // Color based on frequency (low = red, mid = green, high = blue)
      const hue = (i / fftData.length) * 240; // 0 = red, 240 = blue
      const saturation = 70;
      const lightness = Math.min(50 + magnitude * 50, 80);

      ctx.fillStyle = `hsl(${hue}, ${saturation}%, ${lightness}%)`;
      ctx.fillRect(i * barWidth, h - barHeight, barWidth - 1, barHeight);
    }

    // Draw grid overlay
    ctx.strokeStyle = "rgba(255, 255, 255, 0.1)";
    ctx.lineWidth = 0.5;

    // Horizontal frequency lines
    for (let i = 1; i < 8; i++) {
      const y = (h / 8) * i;
      ctx.beginPath();
      ctx.moveTo(0, y);
      ctx.lineTo(w, y);
      ctx.stroke();
    }

    // Vertical time lines
    for (let i = 1; i < 16; i++) {
      const x = (w / 16) * i;
      ctx.beginPath();
      ctx.moveTo(x, 0);
      ctx.lineTo(x, h);
      ctx.stroke();
    }
  };

  return (
    <div className="relative w-full h-full bg-black rounded-lg border-2 border-gray-600 shadow-inner overflow-hidden">
      {/* Display frame */}
      <div className="absolute inset-1 bg-gradient-to-br from-gray-800 to-black rounded border border-gray-700">
        <canvas
          ref={canvasRef}
          className="w-full h-full rounded"
          style={{
            background: "linear-gradient(135deg, #1a1a1a 0%, #0a0a0a 100%)",
          }}
        />

        {/* Frequency labels overlay */}
        <div className="absolute left-2 top-1 text-xs text-gray-400 pointer-events-none">
          <div className="mb-4">20kHz</div>
          <div className="mb-4">10kHz</div>
          <div className="mb-4">5kHz</div>
          <div className="mb-4">2kHz</div>
          <div className="mb-4">1kHz</div>
          <div className="mb-4">500Hz</div>
          <div className="mb-4">250Hz</div>
          <div>20Hz</div>
        </div>

        {/* Time indicator */}
        <div className="absolute bottom-1 right-2 text-xs text-gray-400 pointer-events-none">
          Spectrum Analyzer
        </div>
      </div>
    </div>
  );
}
