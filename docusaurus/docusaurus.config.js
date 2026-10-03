// @ts-check
/** @type {import('@docusaurus/types').Config} */
const config = {
  title: "PWSV Documentation",
  tagline: "High-Performance WebSocket DAW Telemetry & MIDI Streaming VST3 / CLAP Plugin",
  favicon: "img/favicon.ico",

  url: "https://havaianasdestruido.github.io",
  baseUrl: "/docs/",

  organizationName: "havaianasdestruido",
  projectName: "PWSV",

  onBrokenLinks: "throw",

  markdown: {
    hooks: {
      onBrokenMarkdownLinks: "warn",
    },
  },

  i18n: {
    defaultLocale: "en",
    locales: ["en"],
  },

  presets: [
    [
      "classic",
      /** @type {import('@docusaurus/preset-classic').Options} */
      ({
        docs: {
          routeBasePath: "/",
          sidebarPath: "./sidebars.js",
          editUrl: "https://github.com/havaianasdestruido/PWSV/tree/main/docusaurus/",
        },
        blog: false,
        theme: {
          customCss: "./src/css/custom.css",
        },
      }),
    ],
  ],

  themeConfig:
    /** @type {import('@docusaurus/preset-classic').ThemeConfig} */
    ({
      navbar: {
        title: "PWSV Docs",
        logo: {
          alt: "PWSV Logo",
          src: "img/logo.svg",
          href: "/docs/",
        },
        items: [
          {
            href: "pathname:///",
            label: "← Main Site",
            position: "left",
            target: "_self",
          },
          {
            type: "docSidebar",
            sidebarId: "docsSidebar",
            position: "left",
            label: "Documentation",
          },
          {
            to: "/protocol/overview",
            label: "Protocol v2",
            position: "left",
          },
          {
            to: "/cpp-api/",
            label: "C++ API",
            position: "left",
          },
          {
            to: "/client-sdks/overview",
            label: "Client SDKs",
            position: "left",
          },
          {
            href: "https://github.com/havaianasdestruido/PWSV",
            label: "GitHub",
            position: "right",
          },
        ],
      },
      footer: {
        style: "dark",
        links: [
          {
            title: "Docs",
            items: [
              {
                label: "Quick Start",
                to: "/getting-started/quickstart",
              },
              {
                label: "Architecture Overview",
                to: "/architecture/overview",
              },
              {
                label: "Protocol v2 Specification",
                to: "/protocol/overview",
              },
              {
                label: "C++ API Reference",
                to: "/cpp-api/",
              },
            ],
          },
          {
            title: "Ecosystem & Integrations",
            items: [
              {
                label: "Web Browser Monitor",
                to: "/client-sdks/javascript-web",
              },
              {
                label: "Python Terminal Visualizer",
                to: "/client-sdks/terminal-visualizer",
              },
              {
                label: "TouchDesigner",
                to: "/integrations/touchdesigner",
              },
              {
                label: "OBS Studio Overlays",
                to: "/integrations/obs-streaming",
              },
            ],
          },
          {
            title: "Project & Community",
            items: [
              {
                label: "Main Landing Page",
                href: "pathname:///",
              },
              {
                label: "GitHub Repository",
                href: "https://github.com/havaianasdestruido/PWSV",
              },
              {
                label: "Installer Guide",
                to: "/build-and-dev/installer",
              },
              {
                label: "Troubleshooting & FAQ",
                to: "/faq-and-troubleshooting",
              },
            ],
          },
        ],
        copyright: `Copyright © ${new Date().getFullYear()} PWSV (Pato's WebSocket VST). Built with Docusaurus & Jekyll.`,
      },
      prism: {
        theme: require('prism-react-renderer').themes.github,
        darkTheme: require('prism-react-renderer').themes.dracula,
        additionalLanguages: ['cpp', 'bash', 'json', 'cmake', 'python', 'yaml', 'batch'],
      },
      colorMode: {
        defaultMode: 'dark',
        disableSwitch: false,
        respectPrefersColorScheme: true,
      },
    }),
};

module.exports = config;
