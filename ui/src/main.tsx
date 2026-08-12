import { StrictMode } from 'react'
import { createRoot } from 'react-dom/client'
import App from './App.tsx'
import { webViewDspBridge } from './lib/webViewDspBridge'
import './styles/globals.css'

createRoot(document.getElementById('root')!).render(
  <StrictMode>
    <App dspBridge={webViewDspBridge} />
  </StrictMode>,
)
