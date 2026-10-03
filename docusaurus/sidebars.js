/** @type {import('@docusaurus/plugin-content-docs').SidebarsConfig} */
const sidebars = {
  docsSidebar: [
    {
      type: 'category',
      label: 'Getting Started',
      collapsed: false,
      items: [
        'index',
        'getting-started/quickstart',
        'getting-started/installation',
        'getting-started/daw-setup',
      ],
    },
    {
      type: 'category',
      label: 'Core Architecture',
      collapsed: false,
      items: [
        'architecture/overview',
        'architecture/audio-processor',
        'architecture/websocket-server',
        'architecture/gui',
      ],
    },
    {
      type: 'category',
      label: 'Plugins & Parameters',
      collapsed: false,
      items: [
        'plugins/effect-vs-generator',
        'plugins/parameters',
      ],
    },
    {
      type: 'category',
      label: 'WebSocket Protocol v2',
      collapsed: false,
      items: [
        'protocol/overview',
        'protocol/framing-and-handshake',
        'protocol/json-schema',
        'protocol/timing-modes',
      ],
    },
    {
      type: 'category',
      label: 'C++ API Reference',
      collapsed: false,
      items: [
        'cpp-api/index',
        'cpp-api/websocket-server',
        'cpp-api/websocket-processor-base',
        'cpp-api/effect-processor',
        'cpp-api/generator-processor',
        'cpp-api/plugin-editor',
        'cpp-api/position-data',
        'cpp-api/cryptography',
      ],
    },
    {
      type: 'category',
      label: 'Client SDKs & Samples',
      collapsed: false,
      items: [
        'client-sdks/overview',
        'client-sdks/javascript-web',
        'client-sdks/python',
        'client-sdks/terminal-visualizer',
      ],
    },
    {
      type: 'category',
      label: 'Integrations & Tutorials',
      collapsed: true,
      items: [
        'integrations/touchdesigner',
        'integrations/obs-streaming',
        'integrations/unreal-unity',
        'integrations/max-msp-puredata',
      ],
    },
    {
      type: 'category',
      label: 'Build System & DevOps',
      collapsed: true,
      items: [
        'build-and-dev/cmake',
        'build-and-dev/installer',
        'build-and-dev/ci-cd',
      ],
    },
    {
      type: 'doc',
      id: 'faq-and-troubleshooting',
      label: 'FAQ & Troubleshooting',
    },
    {
      type: 'doc',
      id: 'changelog',
      label: 'Changelog & Releases',
    },
  ],
};

module.exports = sidebars;
