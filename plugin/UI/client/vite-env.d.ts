/// <reference types="vite/client" />

// Extend the Window interface to include the webview property
interface Window {
  chrome?: {
    webview?: {
      postMessage: (message: any) => void;
      addEventListener: (
        type: string,
        listener: (event: MessageEvent) => void
      ) => void;
      removeEventListener: (
        type: string,
        listener: (event: MessageEvent) => void
      ) => void;
    };
  };
}

declare const window: Window;
