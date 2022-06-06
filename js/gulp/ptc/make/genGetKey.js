const fs = require('fs');
const dotenv = require('dotenv');

module.exports = function genGetKey(genkeySpec = null) {
  let production = {};
  let productionLocal = {};
  let productionSpec = {};
  if (genkeySpec && fs.existsSync(genkeySpec)) {
    let b = fs.readFileSync(genkeySpec);
    productionSpec = dotenv.parse(b);
    console.log(productionSpec);
  }
  if (fs.existsSync('.env.production.local')) {
    let b = fs.readFileSync('.env.production.local');
    productionLocal = dotenv.parse(b);
  }
  if (fs.existsSync('.env.production')) {
    let b = fs.readFileSync('.env.production');
    production = dotenv.parse(b);
  }
  let getKey = (key) => productionSpec[key] || productionLocal[key] || production[key];
  return getKey;
}
