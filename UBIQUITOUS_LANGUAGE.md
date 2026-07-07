# Ubiquitous Language

## UI Components

| Term | Definition | Aliases to avoid |
| --- | --- | --- |
| **Knob** | A rotary control that the user drags to adjust a parameter value | Manopola, slider, dial, rotary |
| **Panel** | A visually distinct section of the Editor that groups related controls | Section, zone, module |
| **Rack Ear** | A decorative strip on the left and right edges of the Editor that evokes hardware rack mounting | Side strip, border |
| **Top Bar** | The horizontal area at the top of the Editor containing the Bypass toggle and the Oscilloscope | Header, fascia superiore |
| **Side Bar** | A vertical strip on the left (Input) or right (Output) edge containing a Meter above a Gain Knob | Lateral bar, meter strip |
| **Zone** | A sub-section within the lower area of the Editor, below the Distortion Panel (Chorus, Delay, Coming Soon) | Sub-panel, cell |

## Visual Effects

| Term | Definition | Aliases to avoid |
| --- | --- | --- |
| **LED Arc** | The curved indicator on a Knob that fills proportionally to the parameter value | Arc, indicator arc, progress ring |
| **Glow** | A soft luminous halo rendered around an element (LED Arc, Oscilloscope line, Bypass LED) | Bloom, shine, aura |
| **Grain** | A subtle animated noise texture overlaid on the entire Editor to evoke analog material | Noise overlay, film grain, texture |
| **Scanlines** | Faint horizontal lines overlaid on the Oscilloscope area to evoke a vintage CRT display | CRT lines, horizontal lines |
| **Trail** | A semi-transparent copy of a previous Oscilloscope frame that fades over time, creating a motion blur effect | Ghost, phantom, afterimage |
| **Breath** | A gentle pulsing animation (opacity or thickness) applied to an idle element to signal a standby state | Pulse, idle animation, heartbeat |

## Animation System

| Term | Definition | Aliases to avoid |
| --- | --- | --- |
| **AnimationController** | The single central object that owns the VBlankAnimatorUpdater and all ValueAnimators for the Editor | Animator manager, animation system |
| **VBlank** | The vertical blanking interval — the moment between monitor frames used to synchronize rendering | Refresh tick, frame sync |
| **ValueAnimator** | An object that interpolates a numeric value over time along a defined easing curve | Tween, interpolator, animator |
| **Easing** | The mathematical curve that controls the rate of change of an animation (linear, spring, bezier) | Curve, timing function |
| **Spring** | An easing curve that overshoots the target value and settles back, simulating physical elasticity | Bounce, elastic, overshoot |
| **Transition** | A coordinated animation of multiple elements triggered by a state change (e.g. Bypass toggle) | State animation, scene change |

## Audio-to-UI Bridge

| Term | Definition | Aliases to avoid |
| --- | --- | --- |
| **Ring Buffer** | A lock-free circular buffer (juce::AbstractFifo) that transfers audio samples from the Processor to the Editor without blocking the audio thread | Circular buffer, FIFO, audio queue |
| **RMS** | Root Mean Square — the average signal level over a short window, displayed as the main Meter bar height | Average level, volume |
| **Peak** | The instantaneous maximum signal amplitude, displayed as the Peak Hold indicator above the Meter bar | Max level, transient |
| **Peak Hold** | A horizontal marker on a Meter that jumps to the current Peak instantly and descends slowly with easing | Peak indicator, max marker |

## Signal Visualization

| Term | Definition | Aliases to avoid |
| --- | --- | --- |
| **Oscilloscope** | The waveform display component in the Top Bar that renders the audio signal as an animated Path | Waveform, scope, signal view |
| **Input Signal** | The audio waveform captured before the DSP chain, rendered as a light/transparent violet line | Pre-FX, dry signal |
| **Output Signal** | The audio waveform captured after the DSP chain, rendered as a solid violet line | Post-FX, wet signal |
| **Flatline** | The visual state of the Output Signal fading to grey and flattening when Bypass is engaged | Dead signal, greyed out |

## Layout Regions

| Term | Definition | Aliases to avoid |
| --- | --- | --- |
| **Distortion Panel** | The large central Panel in the upper area showing COLOR and BITCRUSH knobs | Main panel, drive section |
| **Chorus Zone** | The lower-left Zone containing the CHORUS MIX knob | Modulation zone |
| **Delay Zone** | The lower-center Zone containing DELAY TIME, DELAY MIX, and DELAY FEEDBACK knobs | Echo zone |
| **Coming Soon Zone** | The lower-right Zone reserved as a placeholder for a future creative effect | Empty zone, placeholder, future slot |

## Visual Identity

| Term | Definition | Aliases to avoid |
| --- | --- | --- |
| **Palette** | The set of canonical colors: cream/ivory background, violet accents, with supporting greys and whites | Color scheme, theme |
| **LookAndFeel** | The JUCE class that defines how all standard UI elements (knobs, buttons, labels) are drawn and styled | Theme, skin, style |
| **Typography** | The two-font system: Bebas Neue for Panel titles, Montserrat for labels and values | Fonts, type |

## Relationships

- The **AnimationController** owns exactly one **VBlank** updater and zero or more **ValueAnimators**
- A **Knob** has one **LED Arc** and may have a **Glow**
- A **Side Bar** contains one **Meter** and one Gain **Knob**
- A **Meter** displays **RMS** as its bar and **Peak Hold** as its marker
- The **Oscilloscope** renders an **Input Signal** and an **Output Signal**, each with zero or more **Trails**
- A **Transition** orchestrates multiple **ValueAnimators** simultaneously
- The **Ring Buffer** connects the Processor (audio thread) to the **Oscilloscope** and **Meters** (UI thread)

## Example dialogue

> **Dev:** "When the user toggles Bypass, should the **Oscilloscope** stop immediately?"
> **Domain expert:** "No — trigger a **Transition**. The **Output Signal** fades to grey and becomes a **Flatline**, while the **Input Signal** keeps rendering normally. The **Trails** should dissolve slowly."
> **Dev:** "And the **Knobs** — do they freeze visually?"
> **Domain expert:** "The **LED Arcs** dim via their **ValueAnimators**, and the **Glows** fade out. Use a **Spring** easing on the Bypass **Switch** itself so the lever overshoots satisfyingly."
> **Dev:** "Got it. The **Ring Buffer** keeps feeding samples the whole time?"
> **Domain expert:** "Exactly. The Processor doesn't know about the UI state. The **AnimationController** decides what to render based on the Bypass parameter value."

## Flagged ambiguities

- "Zone" and "Panel" were both used to describe regions of the Editor. Canonical distinction: a **Panel** is a major region with its own background (Distortion Panel), while a **Zone** is a sub-region within the lower area that may or may not have a distinct background (Chorus Zone, Delay Zone, Coming Soon Zone).
- "Modulation" was used in the old UI to label the panel containing Chorus. In the new layout, the term is dropped in favor of **Chorus Zone** to avoid confusion with the DSP concept of modulation.
- "Gain" was used both for the parameter (INPUT_GAIN, OUTPUT_GAIN) and for the old panel name. In the new layout, Gain controls live in the **Side Bars** and the old "Gain Panel" no longer exists.
