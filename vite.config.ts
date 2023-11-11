import { defineConfig } from 'vite';

// https://vitejs.dev/config/
export default defineConfig({
  plugins: [],
  server: {
    headers: {
      "content-security-policy": "base-uri 'self'; font-src 'self'; form-action 'self'; frame-ancestors 'self'; img-src 'self'; object-src 'none'; script-src 'self' 'wasm-unsafe-eval' blob:; style-src 'self' 'unsafe-inline'",
      "cross-origin-embedder-policy": "require-corp",
      "cross-origin-opener-policy": "same-site",
      "cross-origin-resource-policy": "same-site",
      "cross-agent-cluster": "?1",
      "referrer-policy": "no-referrer",
      "x-content-type-options": "no-sniff",
      "x-dns-prefetch-control": "off",
      "x-download-options": "noopen",
      "x-frame-options": "SAMEORIGIN",
      "x-permitted-cross-domain-policies": "none",
      "x-xss-protection": "0",
      "Access-Control-Allow-Origin": "http://localhost:5173",
    }
  }
});
