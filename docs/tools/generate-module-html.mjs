#!/usr/bin/env node

import fs from "node:fs";
import path from "node:path";

const root = path.resolve(new URL("../..", import.meta.url).pathname);

const modules = fs
  .readdirSync(root, { withFileTypes: true })
  .filter((entry) => entry.isDirectory())
  .map((entry) => entry.name)
  .filter((name) => /^\d+-/.test(name))
  .filter((name) => fs.existsSync(path.join(root, name, "README.md")))
  .sort((a, b) => a.localeCompare(b, undefined, { numeric: true }));

const moduleMetadata = {
  "0-LedToggling": {
    peripheral: "GPIO output",
    tag: "GPIO Output",
    summary: "Enable GPIOA and toggle PA5 / LD2",
    card: "Enables GPIOA and toggles PA5, which drives the Nucleo LD2 user LED.",
  },
  "1-ButtonLedControl": {
    peripheral: "GPIO input/output",
    tag: "GPIO Input/Output",
    summary: "Read PC13 user button and drive PA5 LED",
    card: "Reads PC13 user button input and controls PA5 LED output. The button is active-low.",
  },
  "2-UartTx": {
    peripheral: "USART2 transmit",
    tag: "USART2 TX",
    summary: "Send ASCII `A` on PA2 at 9600 baud",
    card: "Configures USART2 on PA2/PA3 and transmits ASCII data at 9600 baud.",
  },
  "3-ADC": {
    peripheral: "ADC1",
    tag: "ADC1",
    summary: "Read internal temperature sensor channel 16",
    card: "Enables ADC1 and reads the internal temperature sensor channel.",
  },
  "4-UartTxRx": {
    peripheral: "USART2 transmit/receive",
    tag: "USART2 TX/RX",
    summary: "Receive terminal text on PA3 and echo it on PA2",
    card: "Receives a terminal line on PA3 and echoes it back on PA2 with a simple blocking driver.",
  },
};

function escapeHtml(value) {
  return value
    .replaceAll("&", "&amp;")
    .replaceAll("<", "&lt;")
    .replaceAll(">", "&gt;")
    .replaceAll('"', "&quot;");
}

function inlineMarkdown(value) {
  let html = escapeHtml(value);
  html = html.replace(/`([^`]+)`/g, "<code>$1</code>");
  html = html.replace(/\*\*([^*]+)\*\*/g, "<strong>$1</strong>");
  html = html.replace(/!\[([^\]]*)\]\(([^)]+)\)/g, '<img class="proof" src="$2" alt="$1">');
  html = html.replace(/\[([^\]]+)\]\(([^)]+)\)/g, '<a href="$2">$1</a>');
  return html;
}

function replaceGeneratedBlock(content, startMarker, endMarker, replacement) {
  const pattern = new RegExp(
    `(${startMarker.replace(/[.*+?^${}()|[\]\\]/g, "\\$&")})[\\s\\S]*?(${endMarker.replace(/[.*+?^${}()|[\]\\]/g, "\\$&")})`,
  );

  if (!pattern.test(content)) {
    throw new Error(`Could not find generated block: ${startMarker} ... ${endMarker}`);
  }

  return content.replace(pattern, `$1\n${replacement}\n$2`);
}

function readFirstParagraph(moduleName) {
  const markdown = fs.readFileSync(path.join(root, moduleName, "README.md"), "utf8");
  const lines = markdown.split(/\r?\n/);
  const paragraph = [];

  for (const line of lines) {
    if (!line.trim() || line.startsWith("#")) {
      if (paragraph.length) break;
      continue;
    }
    if (line.startsWith("```") || line.startsWith("|") || line.startsWith("![")) break;
    paragraph.push(line.trim());
  }

  return paragraph.join(" ") || "Standalone bare-metal STM32 peripheral module.";
}

function getModuleInfo(moduleName) {
  const metadata = moduleMetadata[moduleName] ?? {};
  const fallbackName = moduleName.replace(/^\d+-/, "").replace(/-/g, " ");
  const firstParagraph = readFirstParagraph(moduleName);

  return {
    name: moduleName,
    peripheral: metadata.peripheral ?? fallbackName,
    tag: metadata.tag ?? fallbackName,
    summary: metadata.summary ?? firstParagraph,
    card: metadata.card ?? firstParagraph,
  };
}

function renderReadmeModuleIndex() {
  const rows = modules.map((moduleName) => {
    const info = getModuleInfo(moduleName);
    return `| \`${moduleName}\` | ${info.peripheral} | ${info.summary} | [README](${moduleName}/README.md) | [HTML](${moduleName}/index.html) |`;
  });

  return [
    "| Project | Peripheral | What It Demonstrates | README | HTML Guide |",
    "| --- | --- | --- | --- | --- |",
    ...rows,
  ].join("\n");
}

function renderHtmlModuleCards() {
  return `      <div class="grid">
${modules
  .map((moduleName) => {
    const info = getModuleInfo(moduleName);
    return `        <article class="project">
          <span class="tag">${escapeHtml(info.tag)}</span>
          <h3>${escapeHtml(moduleName)}</h3>
          <p>${escapeHtml(info.card)}</p>
          <p><a href="${moduleName}/">Open HTML guide</a> | <a href="${moduleName}/README.md">README</a></p>
        </article>`;
  })
  .join("\n\n")}
      </div>`;
}

function renderTable(lines) {
  const rows = lines
    .filter((line) => /^\|.*\|$/.test(line.trim()))
    .filter((line) => !/^\|\s*:?-+:?\s*(\|\s*:?-+:?\s*)+\|?$/.test(line.trim()))
    .map((line) =>
      line
        .trim()
        .replace(/^\|/, "")
        .replace(/\|$/, "")
        .split("|")
        .map((cell) => inlineMarkdown(cell.trim())),
    );

  if (!rows.length) return "";

  const [head, ...body] = rows;
  const header = `<thead><tr>${head.map((cell) => `<th>${cell}</th>`).join("")}</tr></thead>`;
  const tbody = `<tbody>${body
    .map((row) => `<tr>${row.map((cell) => `<td>${cell}</td>`).join("")}</tr>`)
    .join("")}</tbody>`;

  return `<table>${header}${tbody}</table>`;
}

function renderMarkdown(markdown) {
  const lines = markdown.split(/\r?\n/);
  const html = [];
  let paragraph = [];
  let code = [];
  let inCode = false;
  let table = [];

  function flushParagraph() {
    if (!paragraph.length) return;
    html.push(`<p>${inlineMarkdown(paragraph.join(" "))}</p>`);
    paragraph = [];
  }

  function flushTable() {
    if (!table.length) return;
    html.push(renderTable(table));
    table = [];
  }

  for (const line of lines) {
    if (line.startsWith("```")) {
      flushParagraph();
      flushTable();
      if (inCode) {
        html.push(`<pre><code>${escapeHtml(code.join("\n"))}</code></pre>`);
        code = [];
        inCode = false;
      } else {
        inCode = true;
      }
      continue;
    }

    if (inCode) {
      code.push(line);
      continue;
    }

    if (/^\|.*\|$/.test(line.trim())) {
      flushParagraph();
      table.push(line);
      continue;
    }

    flushTable();

    if (!line.trim()) {
      flushParagraph();
      continue;
    }

    const heading = line.match(/^(#{1,4})\s+(.+)$/);
    if (heading) {
      flushParagraph();
      const level = Math.min(heading[1].length + 1, 5);
      html.push(`<h${level}>${inlineMarkdown(heading[2])}</h${level}>`);
      continue;
    }

    const image = line.match(/^!\[([^\]]*)\]\(([^)]+)\)$/);
    if (image) {
      flushParagraph();
      html.push(`<img class="proof" src="${escapeHtml(image[2])}" alt="${escapeHtml(image[1])}">`);
      continue;
    }

    const bullet = line.match(/^\s*[-*]\s+(.+)$/);
    if (bullet) {
      flushParagraph();
      html.push(`<ul><li>${inlineMarkdown(bullet[1])}</li></ul>`);
      continue;
    }

    paragraph.push(line.trim());
  }

  flushParagraph();
  flushTable();

  return html.join("\n");
}

function pageTemplate(moduleName, body) {
  const title = moduleName.replace(/-/g, " ");
  return `<!doctype html>
<html lang="en">
<head>
  <meta charset="utf-8">
  <meta name="viewport" content="width=device-width, initial-scale=1">
  <title>${escapeHtml(title)} Documentation</title>
  <style>
    :root {
      color-scheme: light;
      --bg: #f4f7f8;
      --panel: #ffffff;
      --ink: #1c2430;
      --muted: #5c6675;
      --line: #d8e0e7;
      --brand: #0f6674;
      --brand-2: #2e7d61;
      --accent: #b46621;
      --code-bg: #101720;
      --code-ink: #eaf0f4;
      --soft: #eef6f4;
      --shadow: 0 18px 45px rgba(28, 36, 48, 0.08);
    }
    * { box-sizing: border-box; }
    html { scroll-behavior: smooth; }
    body {
      margin: 0;
      font-family: -apple-system, BlinkMacSystemFont, "Segoe UI", Arial, sans-serif;
      line-height: 1.6;
      color: var(--ink);
      background:
        linear-gradient(180deg, #eef5f6 0, var(--bg) 310px),
        var(--bg);
    }
    header {
      background: #12232b;
      color: #fff;
      padding: 42px 22px 34px;
      border-bottom: 5px solid var(--brand-2);
    }
    header .wrap, main { max-width: 1080px; margin: 0 auto; }
    h1 { margin: 0 0 8px; font-size: clamp(30px, 4vw, 44px); line-height: 1.12; letter-spacing: 0; }
    header p { margin: 0; color: #d7e4ea; }
    main { padding: 24px 18px 52px; }
    nav {
      background: var(--panel);
      border: 1px solid var(--line);
      border-radius: 8px;
      padding: 12px 14px;
      margin-bottom: 18px;
      box-shadow: var(--shadow);
    }
    a { color: var(--brand); font-weight: 700; text-decoration-thickness: 1px; text-underline-offset: 3px; }
    a:hover { color: var(--accent); }
    section {
      background: var(--panel);
      border: 1px solid var(--line);
      border-radius: 8px;
      padding: clamp(18px, 3vw, 30px);
      box-shadow: var(--shadow);
    }
    h2 { margin: 28px 0 10px; color: #12313f; letter-spacing: 0; border-top: 1px solid var(--line); padding-top: 22px; }
    h2:first-of-type { border-top: 0; padding-top: 0; }
    h3, h4, h5 { margin: 22px 0 8px; letter-spacing: 0; }
    p { max-width: 82ch; }
    ul { padding-left: 22px; }
    li { margin: 4px 0; }
    table { width: 100%; border-collapse: collapse; margin: 14px 0 18px; font-size: 14px; border-radius: 8px; overflow: hidden; }
    th, td { border: 1px solid var(--line); padding: 10px 12px; text-align: left; vertical-align: top; }
    th { background: #e7f0f1; color: #163440; }
    tr:nth-child(even) td { background: #fbfcfd; }
    code { font-family: "SFMono-Regular", Consolas, "Liberation Mono", monospace; font-size: 0.94em; }
    :not(pre) > code { background: #edf3f5; color: #12313f; padding: 2px 5px; border-radius: 5px; }
    pre {
      overflow-x: auto;
      background: var(--code-bg);
      color: var(--code-ink);
      padding: 16px;
      border-radius: 8px;
      border: 1px solid #0b1220;
      box-shadow: inset 0 1px 0 rgba(255,255,255,0.05);
    }
    pre code { color: inherit; }
    .proof {
      max-width: 100%;
      border: 1px solid var(--line);
      border-radius: 8px;
      margin: 10px 0 18px;
      background: #000;
      box-shadow: 0 12px 30px rgba(28, 36, 48, 0.12);
    }
    .note {
      color: #334155;
      font-size: 14px;
      background: var(--soft);
      border-left: 4px solid var(--brand-2);
      padding: 12px 14px;
      border-radius: 6px;
    }
    @media (max-width: 720px) {
      header { padding: 30px 18px 24px; }
      main { padding: 16px 12px 40px; }
      table { display: block; overflow-x: auto; }
    }
  </style>
</head>
<body>
  <header>
    <div class="wrap">
      <h1>${escapeHtml(moduleName)}</h1>
      <p>Standalone HTML documentation generated from this module README.</p>
    </div>
  </header>
  <main>
    <nav>
      <a href="../">Main documentation</a> |
      <a href="README.md">Markdown README</a>
    </nav>
    <section>
      <p class="note">Generated by <code>docs/tools/generate-module-html.mjs</code>. Edit <code>README.md</code>, then run the generator to refresh <code>index.html</code> and <code>documentation.html</code>.</p>
${body}
    </section>
  </main>
</body>
</html>
`;
}

for (const moduleName of modules) {
  const readmePath = path.join(root, moduleName, "README.md");
  const documentationPath = path.join(root, moduleName, "documentation.html");
  const indexPath = path.join(root, moduleName, "index.html");
  const markdown = fs.readFileSync(readmePath, "utf8");
  const html = pageTemplate(moduleName, renderMarkdown(markdown));
  fs.writeFileSync(documentationPath, html);
  fs.writeFileSync(indexPath, html);
  console.log(`Generated ${path.relative(root, documentationPath)}`);
  console.log(`Generated ${path.relative(root, indexPath)}`);
}

const readmePath = path.join(root, "README.md");
const readme = fs.readFileSync(readmePath, "utf8");
fs.writeFileSync(
  readmePath,
  replaceGeneratedBlock(readme, "<!-- MODULE_INDEX_START -->", "<!-- MODULE_INDEX_END -->", renderReadmeModuleIndex()),
);
console.log("Updated README.md module index");

const mainHtmlPath = path.join(root, "documentation.html");
const mainHtml = fs.readFileSync(mainHtmlPath, "utf8");
fs.writeFileSync(
  mainHtmlPath,
  replaceGeneratedBlock(mainHtml, "<!-- MODULE_CARDS_START -->", "<!-- MODULE_CARDS_END -->", renderHtmlModuleCards()),
);
console.log("Updated documentation.html module cards");

fs.copyFileSync(mainHtmlPath, path.join(root, "index.html"));
console.log("Updated index.html GitHub Pages entry point");

console.log(`Generated ${modules.length} module HTML page(s).`);
