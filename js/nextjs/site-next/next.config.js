/** @type {import('next').NextConfig} */
const withYAML = require('next-yaml')
const path = require('path')

module.exports = {
  reactStrictMode: true,
  images: {
    disableStaticImages: true,
  },
  webpack: (config, { buildId, dev, isServer, defaultLoaders, webpack }) => {
    config.plugins.push(new webpack.ProvidePlugin({
      $: 'jquery',
      jQuery: 'jquery',
      'window.jQuery': 'jquery'
    }))
    return config;
  },
  sassOptions: {
    includePaths: [path.join(__dirname, 'scss')],
  },
}

module.exports = withYAML(module.exports)



