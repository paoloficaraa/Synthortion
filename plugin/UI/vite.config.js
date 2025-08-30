import { defineConfig } from "vite";
import react from "@vitejs/plugin-react";
import path from "path";
export default defineConfig({
    plugins: [react()],
    resolve: {
        alias: {
            "@": path.resolve(__dirname, "./client"),
            "~": path.resolve(__dirname, "./"),
            juce: path.resolve(__dirname, "../../libs/juce/modules/juce_gui_extra/native/javascript"),
        },
    },
    root: ".",
    build: {
        outDir: "dist",
        emptyOutDir: true,
    },
    server: {
        port: 5173,
        host: true,
    },
});
