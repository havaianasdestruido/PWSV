// PWSV Interactive Homepage Simulator & UI Logic

document.addEventListener('DOMContentLoaded', () => {
  // ── Tab switcher ──────────────────────────────────────────
  const tabBtns = document.querySelectorAll('.code-tab-btn');
  const tabPanes = document.querySelectorAll('.code-tab-pane');

  tabBtns.forEach(btn => {
    btn.addEventListener('click', () => {
      tabBtns.forEach(b => b.classList.remove('active'));
      tabPanes.forEach(p => p.classList.remove('active'));
      
      btn.classList.add('active');
      const targetId = btn.getAttribute('data-tab');
      const targetPane = document.getElementById(targetId);
      if (targetPane) {
        targetPane.classList.add('active');
      }
    });
  });

  // ── Live Simulator Engine ─────────────────────────────────
  let isPlaying = true;
  let isRecording = false;
  let isLooping = true;
  let bpm = 128.0;
  let mode = 0; // 0: Hz, 1: Beats, 2: Minutes
  let rateHz = 15;
  let activeNotes = [
    { note: 60, vel: 100, ch: 1 },
    { note: 64, vel: 95, ch: 1 }
  ];

  let timeSec = 32.450;
  let timeSamples = 1431045;
  let ppq = 69.226;
  let bar = 18;
  let beat = 1.22;

  const jsonDisplay = document.getElementById('demo-json-output');
  const bpmSlider = document.getElementById('demo-bpm-slider');
  const bpmVal = document.getElementById('demo-bpm-val');
  const playBtn = document.getElementById('demo-play-btn');
  const loopBtn = document.getElementById('demo-loop-btn');
  const pianoKeys = document.querySelectorAll('.piano-key');

  if (bpmSlider && bpmVal) {
    bpmSlider.addEventListener('input', (e) => {
      bpm = parseFloat(e.target.value);
      bpmVal.textContent = `${bpm.toFixed(0)} BPM`;
    });
  }

  if (playBtn) {
    playBtn.addEventListener('click', () => {
      isPlaying = !isPlaying;
      playBtn.classList.toggle('active', isPlaying);
      playBtn.innerHTML = isPlaying 
        ? '<span>▶ Playing</span>' 
        : '<span>⏸ Paused</span>';
    });
  }

  if (loopBtn) {
    loopBtn.addEventListener('click', () => {
      isLooping = !isLooping;
      loopBtn.classList.toggle('active', isLooping);
    });
  }

  if (pianoKeys) {
    pianoKeys.forEach(key => {
      key.addEventListener('click', () => {
        const noteNum = parseInt(key.getAttribute('data-note'), 10);
        const existingIdx = activeNotes.findIndex(n => n.note === noteNum);
        
        if (existingIdx >= 0) {
          activeNotes.splice(existingIdx, 1);
          key.classList.remove('active');
        } else {
          activeNotes.push({ note: noteNum, vel: 100 + Math.floor(Math.random() * 20), ch: 1 });
          key.classList.add('active');
        }
      });
    });
  }

  // Simulation tick
  setInterval(() => {
    if (isPlaying) {
      const dt = 0.05; // 50ms
      timeSec += dt;
      timeSamples += Math.floor(dt * 44100);
      const beatsPerSec = bpm / 60.0;
      ppq += dt * beatsPerSec;
      
      bar = Math.floor(ppq / 4.0) + 1;
      beat = (ppq % 4.0);
    }

    if (jsonDisplay) {
      const payload = {
        protocol: 2,
        time_sec: parseFloat(timeSec.toFixed(3)),
        time_samples: timeSamples,
        ppq: parseFloat(ppq.toFixed(3)),
        bpm: parseFloat(bpm.toFixed(1)),
        bar: bar,
        beat: parseFloat(beat.toFixed(2)),
        time_sig: [4, 4],
        playing: isPlaying,
        recording: isRecording,
        looping: isLooping,
        notes: activeNotes
      };

      jsonDisplay.textContent = JSON.stringify(payload, null, 2);
    }
  }, 100);
});
