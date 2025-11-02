# Frontend

React Frontend für das FullBlown MoRa-LC System.

## Stack

- **React 18** + **Vite**
- **TailwindCSS** - Styling
- **Leaflet** - Karten
- **Socket.io-client** - Live-Updates
- **Recharts** - Diagramme
- **React Router** - Navigation

## Setup

```bash
# Dependencies installieren
npm install

# Development Server
npm run dev

# Build für Production
npm run build

# Preview Production Build
npm run preview
```

Dev-Server: `http://localhost:5173`

## Struktur

```
/src/
  /components/          # Wiederverwendbare UI-Komponenten
    TrackMap.jsx        # Leaflet Karte
    CheckpointEditor.jsx # Drag-Drop Checkpoints
    LiveTelemetry.jsx   # Echtzeit-Daten
    LapTimeTable.jsx    # Rundenzeiten-Tabelle
    TrafficLight.jsx    # Display-Steuerung
    TeamCard.jsx        # Team-Karte
    Button.jsx          # UI Buttons
    Modal.jsx           # Modal-Dialoge
    
  /views/               # Seiten/Views
    AdminDashboard.jsx  # Admin-Kontrolle
    LiveView.jsx        # Öffentliche Live-Ansicht
    TeamView.jsx        # Team-Login & Auswertung
    AnalysisView.jsx    # Multi-Rennen Analyse
    SetupView.jsx       # Ersteinrichtung
    
  /services/            # API & Services
    api.js              # REST API Client
    websocket.js        # WebSocket Manager
    auth.js             # Authentifizierung
    
  /utils/               # Hilfsfunktionen
    rssi_visualization.js # RSSI Heatmap
    time_formatting.js  # Zeit-Formatierung
    export.js           # CSV/JSON Export
    
  /hooks/               # Custom React Hooks
    useWebSocket.js     # WebSocket Hook
    useRaceState.js     # Race State Management
    useBeaconTracking.js # Beacon-Tracking
    
  App.jsx               # App Root
  main.jsx              # Entry Point
  index.css             # Global Styles
```

## Views

### Admin Dashboard (`/admin`)

**Features:**
- Strecken-Setup (Karte, Checkpoints)
- Team & Beacon Verwaltung
- Rennen-Kontrolle (Start/Stop/Pause)
- Live-Monitoring
- Display-Steuerung (Ampel)
- Strafen-Verwaltung

**Route:** `/admin`

### Live View (`/live`)

**Features:**
- Live-Karte mit Fahrzeugpositionen
- Rangliste (Echtzeit)
- Beste Rundenzeiten
- Restzeit-Anzeige

**Route:** `/live` (öffentlich)

### Team View (`/team/:id`)

**Features:**
- Team-spezifische Auswertung
- Alle Rundenzeiten
- Statistiken (Durchschnitt, Beste)
- Vergleich mit anderen Teams
- Geschwindigkeitsprofil

**Route:** `/team/:teamId` (Login erforderlich)

### Analysis View (`/analysis`)

**Features:**
- Multi-Rennen Vergleich
- Saison-Statistiken
- Ranglisten über Zeit
- Export-Funktionen

**Route:** `/analysis`

## API Client

`services/api.js`:

```javascript
import axios from 'axios';

const api = axios.create({
  baseURL: 'http://localhost:8000/api',
  timeout: 5000,
});

// Teams
export const getTeams = () => api.get('/teams');
export const createTeam = (data) => api.post('/teams', data);

// Races
export const getRaces = () => api.get('/races');
export const startRace = (id) => api.put(`/races/${id}/start`);
```

## WebSocket

`services/websocket.js`:

```javascript
import io from 'socket.io-client';

const socket = io('http://localhost:8000');

// Live-Telemetrie empfangen
socket.on('telemetry', (data) => {
  console.log('Beacon update:', data);
});

// Lap-Events
socket.on('lap_completed', (data) => {
  console.log('New lap:', data);
});
```

## Styling

### TailwindCSS

Utility-First CSS Framework.

Beispiel:
```jsx
<button className="px-4 py-2 bg-blue-500 hover:bg-blue-600 text-white rounded">
  Start Rennen
</button>
```

### Custom Theme

`tailwind.config.js`:

```javascript
module.exports = {
  theme: {
    extend: {
      colors: {
        primary: '#3B82F6',
        secondary: '#10B981',
        danger: '#EF4444',
      }
    }
  }
}
```

## State Management

### React Context

Globaler State für Race-Daten, Teams, etc.

```jsx
// RaceContext.jsx
export const RaceContext = createContext();

export function RaceProvider({ children }) {
  const [currentRace, setCurrentRace] = useState(null);
  const [teams, setTeams] = useState([]);
  
  return (
    <RaceContext.Provider value={{ currentRace, teams }}>
      {children}
    </RaceContext.Provider>
  );
}
```

## Development

### Hot Module Replacement

Vite unterstützt HMR out-of-the-box:

```bash
npm run dev
# Änderungen werden sofort im Browser aktualisiert
```

### Environment Variables

`.env.local`:

```
VITE_API_URL=http://localhost:8000
VITE_WS_URL=ws://localhost:8000
VITE_MAP_TILES_URL=https://tile.openstreetmap.org
```

Zugriff:
```javascript
const apiUrl = import.meta.env.VITE_API_URL;
```

### Linting

```bash
# ESLint
npm run lint

# Auto-Fix
npm run lint:fix
```

## Testing

```bash
# Unit Tests (Vitest)
npm run test

# E2E Tests (Playwright)
npm run test:e2e

# Coverage
npm run test:coverage
```

## Build & Deploy

### Production Build

```bash
npm run build
# Output: /dist/
```

### Deploy

**Static Hosting (Netlify, Vercel):**
```bash
# Build Command: npm run build
# Publish Directory: dist
```

**Nginx:**
```nginx
server {
  listen 80;
  server_name mora-lc.local;
  
  root /var/www/mora-lc/dist;
  index index.html;
  
  location / {
    try_files $uri $uri/ /index.html;
  }
  
  location /api {
    proxy_pass http://localhost:8000;
  }
  
  location /socket.io {
    proxy_pass http://localhost:8000;
    proxy_http_version 1.1;
    proxy_set_header Upgrade $http_upgrade;
    proxy_set_header Connection "upgrade";
  }
}
```

## Troubleshooting

**Build fails:**
- Node Version prüfen (>=18)
- `rm -rf node_modules && npm install`

**API calls fail:**
- CORS? Backend muss Frontend-URL erlauben
- `.env.local` korrekt?

**Map doesn't load:**
- Internet-Verbindung für OSM-Tiles
- Alternative: Offline-Tiles

**WebSocket disconnects:**
- Backend läuft?
- Firewall blockiert Port?



