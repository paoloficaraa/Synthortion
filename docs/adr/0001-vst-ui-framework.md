# Research: UI Frameworks for VST Plugins

This document outlines the primary architectural approaches for building a VST plugin Graphical User Interface (GUI), evaluating them against the current project requirements (pixel-perfect Glitch Brutalism design, component reusability, DSP data-binding).

## 1. Web Technologies via WebView (React/Vue/Svelte)
**How it works**: A web UI (HTML/CSS/JS) is bundled and rendered inside the C++ VST wrapper using an embedded browser (like `juce::WebBrowserComponent` or wrappers that use Edge WebView2 / WebKit). Communications with the DSP engine happen via serialized IPC messages (JSON or binary).

*   **Pros**:
    *   **Extremely high UI productivity**: Leverages Vite, React, Tailwind (exactly what is in `ui/`).
    *   **Pixel-Perfect styling**: CSS variables, SVG math, flexbox/grid make complex responsive UIs drastically easier than native C++.
    *   **Rich Ecosystem**: Reusing existing NPM packages.
*   **Cons**:
    *   **Overhead**: An embedded browser has a larger memory footprint.
    *   **Performance**: IPC (Inter-Process Communication) across the JS thread and C++ DSP thread introduces latency. Not ideal for 60fps real-time FFT/waveform rendering unless leveraging WebGL/Canvas directly.

## 2. React Native for JUCE (Blueprint)
**How it works**: Uses the React lifecycle and JS engine (via QuickJS/Duktape), but skips the WebView. Instead of rendering to HTML/DOM, it orchestrates native JUCE C++ UI components.

*   **Pros**: 
    *   React DX with native C++ execution speeds. 
    *   Much lower memory footprint than WebViews.
*   **Cons**: 
    *   CSS support is a subset of React Native (Yoga layout). You cannot seamlessly drop in Tailwind or complex CSS filters. 
    *   Migrating an HTML prototype requires rewriting styling to strict inline objects.

## 3. Native C++ Frameworks (JUCE / iPlug2 / VSTGUI)
**How it works**: Standard industry approach. The GUI is built writing raw C++ overriding paint methods.

*   **Pros**: 
    *   Maximum performance and lowest latency. 
    *   DSP and UI easily share memory (lock-free rings).
*   **Cons**: 
    *   Prototyping speed is slow. 
    *   Achieving "Glitch Brutalism & Dark Industrial" with precise CSS variables, pseudo-selectors, and SVG math requires writing complex custom `LookAndFeel` C++ rendering classes or manual raw OpenGL shaders.

## 4. Rust-based Declarative UIs (Nih-plug + Iced / Baseview)
**How it works**: Rust ecosystem using modern declarative GUI frameworks.

*   **Pros**: Memory safe, highly performant, modern declarative model.
*   **Cons**: Precludes using React + TSX, forcing a complete rewrite in Rust.

---

### Conclusion & Recommendation
Given that there is a **design-perfect HTML prototype** ready and the requirement is **Pixel-Perfect Absolute Visual Fidelity** relying heavily on CSS features (`--bg`, `--surface`, focus-visible), the the **WebView (React + Vite)** approach (Approach #1) is the strongest path forward for the frontend. 

While it sacrifices some memory overhead compared to raw C++, the tradeoff in iteration speed, exact preservation of the SVG math, and direct mapping of the HTML prototype makes it the vastly superior choice for an industrial, stylistically brutalist interface. Real-time data like the `FftVisualizer` can be fed smoothly utilizing WebGL or optimized WebSockets/IPC channels depending on the C++ host wrapper.
