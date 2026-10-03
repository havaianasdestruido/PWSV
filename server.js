const http = require('http');
const fs = require('fs');
const path = require('path');

const PORT = process.env.PORT || 3000;
const HOST = process.env.HOST || '127.0.0.1';

const MIME_TYPES = {
  '.html': 'text/html; charset=utf-8',
  '.css': 'text/css; charset=utf-8',
  '.js': 'application/javascript; charset=utf-8',
  '.json': 'application/json; charset=utf-8',
  '.png': 'image/png',
  '.jpg': 'image/jpeg',
  '.jpeg': 'image/jpeg',
  '.gif': 'image/gif',
  '.svg': 'image/svg+xml',
  '.ico': 'image/x-icon',
  '.woff': 'font/woff',
  '.woff2': 'font/woff2',
  '.ttf': 'font/ttf',
  '.txt': 'text/plain; charset=utf-8',
};

const ROOT_DIR = path.resolve(__dirname);
const DOCS_DIR = path.resolve(__dirname, 'docs');

// Render Jekyll page with layouts, includes and liquid filter emulation
function renderJekyllPage(filePath) {
  let raw = fs.readFileSync(filePath, 'utf8');
  let title = "PWSV — Real-Time WebSocket DAW Telemetry & MIDI VST3/CLAP";
  let description = "Ultra-low latency WebSocket DAW telemetry and polyphonic MIDI streaming VST3 and CLAP audio plugin suite.";
  let content = raw;

  if (raw.startsWith('---')) {
    const parts = raw.split('---');
    if (parts.length >= 3) {
      const frontMatter = parts[1];
      content = parts.slice(2).join('---').trim();

      const titleMatch = frontMatter.match(/title:\s*["']?([^"'\n\r]+)["']?/);
      if (titleMatch) title = titleMatch[1];

      const descMatch = frontMatter.match(/description:\s*["']?([^"'\n\r]+)["']?/);
      if (descMatch) description = descMatch[1];
    }
  }

  // Load layout
  const layoutPath = path.join(__dirname, '_layouts', 'default.html');
  let html = fs.existsSync(layoutPath) ? fs.readFileSync(layoutPath, 'utf8') : '{{ content }}';

  // Replace includes
  const includeRegex = /\{%\s*include\s+([\w\.\-]+)\s*%\}/g;
  html = html.replace(includeRegex, (match, includeFile) => {
    const incPath = path.join(__dirname, '_includes', includeFile);
    if (fs.existsSync(incPath)) {
      return fs.readFileSync(incPath, 'utf8');
    }
    return '';
  });

  // Inject content
  html = html.replace(/\{\{\s*content\s*\}\}/g, content);

  // Replace metadata & filters
  html = html.replace(/\{\%\s*if\s+page\.title\s*\%\}([\s\S]*?)\{\%\s*endif\s*\%\}/g, title ? `${title} | ` : '');
  html = html.replace(/\{\{\s*page\.title\s*\}\}/g, title);
  html = html.replace(/\{\{\s*site\.title\s*\}\}/g, "PWSV — Pato's WebSocket VST");
  html = html.replace(/\{\%\s*if\s+page\.description\s*\%\}[\s\S]*?\{\%\s*else\s*\%\}[\s\S]*?\{\%\s*endif\s*\%\}/g, description);
  html = html.replace(/\{\{\s*site\.repository\s*\}\}/g, "havaianasdestruido/PWSV");
  html = html.replace(/\{\{\s*site\.time\s*\|\s*date:\s*["']%Y["']\s*\}\}/g, new Date().getFullYear().toString());
  
  // Replace relative_url filters
  html = html.replace(/\{\{\s*['"]([^'"]+)['"]\s*\|\s*relative_url\s*\}\}/g, (match, p1) => {
    return p1;
  });

  return html;
}

function serveFile(req, res, filePath) {
  fs.stat(filePath, (err, stats) => {
    if (err || !stats.isFile()) {
      res.writeHead(404, { 'Content-Type': 'text/html' });
      res.end('<h1>404 Not Found</h1>');
      return;
    }

    const ext = path.extname(filePath).toLowerCase();
    const contentType = MIME_TYPES[ext] || 'application/octet-stream';

    res.writeHead(200, {
      'Content-Type': contentType,
      'Content-Length': stats.size,
      'Cache-Control': 'no-cache',
    });

    const stream = fs.createReadStream(filePath);
    stream.pipe(res);
  });
}

const server = http.createServer((req, res) => {
  let urlPath;
  try {
    urlPath = decodeURI(req.url.split('?')[0]);
  } catch (e) {
    res.writeHead(400, { 'Content-Type': 'text/plain; charset=utf-8' });
    res.end('400 Bad Request: Malformed URI');
    return;
  }

  // Route /docs/... to docs directory (Docusaurus build output)
  if (urlPath === '/docs' || urlPath === '/docs/' || urlPath.startsWith('/docs/')) {
    let subPath = urlPath.slice(5); // remove /docs
    if (subPath.startsWith('/')) subPath = subPath.slice(1);
    
    let target = path.resolve(DOCS_DIR, subPath);

    // Containment check
    if (!target.startsWith(DOCS_DIR)) {
      res.writeHead(403, { 'Content-Type': 'text/plain; charset=utf-8' });
      res.end('403 Forbidden');
      return;
    }

    // Check if target is a directory or lacks extension
    if (fs.existsSync(target) && fs.statSync(target).isDirectory()) {
      target = path.join(target, 'index.html');
    } else if (!fs.existsSync(target) && fs.existsSync(target + '.html')) {
      target = target + '.html';
    } else if (!fs.existsSync(target) && fs.existsSync(path.join(target, 'index.html'))) {
      target = path.join(target, 'index.html');
    }

    if (fs.existsSync(target) && target.startsWith(DOCS_DIR)) {
      serveFile(req, res, target);
      return;
    }
  }

  // Primary Jekyll webpage (Static root)
  if (urlPath === '/' || urlPath === '/index.html') {
    const indexPath = path.join(__dirname, 'index.html');
    if (fs.existsSync(indexPath)) {
      const renderedHtml = renderJekyllPage(indexPath);
      const buffer = Buffer.from(renderedHtml, 'utf8');
      res.writeHead(200, {
        'Content-Type': 'text/html; charset=utf-8',
        'Content-Length': buffer.length,
        'Cache-Control': 'no-cache',
      });
      res.end(buffer);
      return;
    }
  }

  let cleanPath = urlPath.startsWith('/') ? urlPath.slice(1) : urlPath;
  let target = path.resolve(ROOT_DIR, cleanPath);

  // Containment check
  if (!target.startsWith(ROOT_DIR)) {
    res.writeHead(403, { 'Content-Type': 'text/plain; charset=utf-8' });
    res.end('403 Forbidden');
    return;
  }

  if (fs.existsSync(target) && fs.statSync(target).isDirectory()) {
    target = path.join(target, 'index.html');
  }

  if (fs.existsSync(target) && target.startsWith(ROOT_DIR)) {
    serveFile(req, res, target);
  } else {
    // 404
    res.writeHead(404, { 'Content-Type': 'text/html' });
    res.end('<h1>404 Not Found</h1>');
  }
});

server.listen(PORT, HOST, () => {
  console.log(`Server running at http://${HOST}:${PORT}/`);
  console.log(`- Primary Jekyll page: http://${HOST}:${PORT}/`);
  console.log(`- Docusaurus docs: http://${HOST}:${PORT}/docs/`);
});
