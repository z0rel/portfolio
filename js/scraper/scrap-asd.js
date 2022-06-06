const Nightmare = require('nightmare');
const cheerio = require('cheerio');
const vo = require('vo');
const fs = require('fs');

function gen_url(num) { return `http://www.deltronics.ru/product/servo/series_57.html?modelPos=${num}&vkl=model-vkl`; }

// Request making using nightmare
// Parsing data using cheerio

vo(run)(function(err, result) {
    if (err) throw err;
});

function* run_urls(nightmare, gen_url, actions, prices, type) {
    let getData = html => {
      const $ = cheerio.load(html);
      $('#model .model-row tbody tr').each((row, raw_element) => {
          if (raw_element.children.length == 1) {
            return;
          }
          let x = $(raw_element).find('td');
          let name = $(x[0]).text().trim();
          let price = $(x[2]).text().trim();
          console.log(name);
          let m = name.match(/[0-9]+([.,][0-9]+)?[ ]*(кВт|kW)/);
          let power = null;
          if (m) {

            power = Number(m[0].replace(/(кВт|kW)/,'').replace(/,/,'.').replace(/[0-9]x/,''));
          }
          let speed = name.match(/[0-9]+об/);
          if (speed) {
            speed = speed[0].replace(/об/, '');
          }
          let features = name.match(/(CANopen|E-Cam|DMCNET|EtherCAT)/i);
          if (features)
            features = features[0];
          let model = name.match(/^[^ ]+/)[0];
          let voltage = name.match(/([0-9][xх])?[0-9]+[ВвV]/);
          let count_outs = null;
          if (voltage) {
            voltage = voltage[0].replace(/[ВвV]/,'');
            count_outs = voltage.match(/^[0-9]x/);
            if (count_outs) {
              count_outs = count_outs[0].replace(/x/, '');
            }
          }
          let valuta = price[price.length-1];
          price = price.substr(0, price.length-1).trim()
          prices.push({ name: name, price: price, valut: valuta, power: power, model: model, voltage: voltage, count_outs: count_outs, speed: speed, features: features, type: type});
      });
    }
    function* asyncRun(i, timeout) {
        yield nightmare
          .goto(gen_url((i*30).toString()))
          .wait('#model .model-row tbody tr')
          .evaluate(
              () => document.querySelector('body').innerHTML
            )
          .run(function(err, response) {
              if (err) throw new Error(err);
              getData(response);
          })
          .wait(timeout);
    }
    for (act of actions) {
        yield asyncRun(act.page, act.timeout);
    }
}

function* run() {
    let nightmare = Nightmare({ show: false});
    let prices = [];
    yield run_urls(nightmare, gen_url, [
      {page: 0, timeout: 8000}, {page: 1, timeout: 1000}, {page: 2, timeout: 1000}, {page: 3, timeout: 1000}, {page: 4, timeout: 1000},
      {page: 5, timeout: 1000}, {page: 6, timeout: 1000}, {page: 7, timeout: 1000}
    ], prices, 'servo');
    yield run_urls(nightmare, (i) => { return "http://www.deltronics.ru/product/CNC/CNC-series_119.html?vkl=model-vkl"; }, [{page: 0, timeout: 3000}], prices, 'servo');
    yield run_urls(nightmare, (i) => { return "http://www.deltronics.ru/product/servo/series_209.html?vkl=model-vkl"; }, [{page: 0, timeout: 3000}], prices, 'servo');
    yield run_urls(nightmare, (i) => { return `http://www.deltronics.ru/product/servo/series_78.html?modelPos=${i}&vkl=model-vkl&accessories=1`; }, [
      {page: 0, timeout: 3000}, {page: 1, timeout: 1000} 
    ], prices, 'accessories');
    yield run_urls(nightmare, (i) => { return "http://www.deltronics.ru/product/CNC/CNC-series_235.html?vkl=model-vkl"; }, [{page: 0, timeout: 3000}], prices, 'cnc');
    yield run_urls(nightmare, (i) => { return "http://www.deltronics.ru/product/CNC/CNC-series_236.html?vkl=model-vkl"; }, [{page: 0, timeout: 3000}], prices, 'cnc');
    yield run_urls(nightmare, (i) => { return "http://www.deltronics.ru/product/CNC/CNC-series_118.html?vkl=model-vkl"; }, [{page: 0, timeout: 3000}], prices, 'cnc');
    yield run_urls(nightmare, (i) => { return "http://www.deltronics.ru/product/CNC/CNC-series_205.html?accessories=1"; }, [{page: 0, timeout: 3000}], prices, 'accesories-dmcnet');
    yield run_urls(nightmare, (i) => { return "http://www.deltronics.ru/product/CNC/CNC-series_121.html?vkl=model-vkl&accessories=1"; }, [{page: 0, timeout: 3000}], prices, 'cnc');
    yield run_urls(nightmare, (i) => { return "http://www.deltronics.ru/product/CNC/CNC-series_120.html?vkl=model-vkl&accessories=1"; }, [{page: 0, timeout: 3000}], prices, 'accesories-cnc');
    yield run_urls(nightmare, (i) => { return "http://www.deltronics.ru/product/encoderi/series_38.html?vkl=model-vkl"; }, [{page: 0, timeout: 3000}], prices, 'encoders');
    yield run_urls(nightmare, (i) => { return "http://www.deltronics.ru/product/fotek/series2_89.html?vkl=model-vkl"; }, [{page: 0, timeout: 3000}], prices, 'encoders');
    yield run_urls(nightmare, (i) => { return "http://www.deltronics.ru/product/fotek/series2_89.html?vkl=model-vkl"; }, [{page: 0, timeout: 3000}], prices, 'encoders');
    yield run_urls(nightmare, (i) => { return "http://www.deltronics.ru/product/istochniki-pitaniya/series_18.html?vkl=model-vkl"; }, [{page: 0, timeout: 3000}], prices, 'sources');
    yield run_urls(nightmare, (i) => { return "http://www.deltronics.ru/product/istochniki-pitaniya/series_206.html?vkl=model-vkl"; }, [{page: 0, timeout: 3000}], prices, 'sources');
    yield run_urls(nightmare, (i) => { return "http://www.deltronics.ru/product/istochniki-pitaniya/series_197.html?vkl=model-vkl"; }, [{page: 0, timeout: 3000}], prices, 'sources');
    yield run_urls(nightmare, (i) => { return `http://www.deltronics.ru/product/istochniki-pitaniya/series_131.html?modelPos=${i}&vkl=model-vkl`; }, [{page: 0, timeout: 3000}, {page: 0, timeout: 1000}], prices, 'sources');
    yield run_urls(nightmare, (i) => { return "http://www.deltronics.ru/product/istochniki-pitaniya/series_128.html?vkl=model-vkl"; }, [{page: 0, timeout: 3000}], prices, 'sources');
    yield run_urls(nightmare, (i) => { return "http://www.deltronics.ru/product/istochniki-pitaniya/series_254.html?vkl=model-vkl"; }, [{page: 0, timeout: 3000}], prices, 'sources');
    yield run_urls(nightmare, (i) => { return "http://www.deltronics.ru/product/istochniki-pitaniya/series_20.html?vkl=model-vkl"; }, [{page: 0, timeout: 3000}], prices, 'sources');
    yield run_urls(nightmare, (i) => { return "http://www.deltronics.ru/product/istochniki-pitaniya/series_19.html?vkl=model-vkl"; }, [{page: 0, timeout: 3000}], prices, 'sources');
    yield run_urls(nightmare, (i) => { return "http://www.deltronics.ru/product/istochniki-pitaniya/series_19.html?modelPos=30&vkl=model-vkl"; }, [{page: 0, timeout: 3000}], prices, 'sources');
    yield run_urls(nightmare, (i) => { return "http://www.deltronics.ru/product/istochniki-pitaniya/series_231.html?vkl=model-vkl"; }, [{page: 0, timeout: 3000}], prices, 'sources');
    yield run_urls(nightmare, (i) => { return "http://www.deltronics.ru/product/istochniki-pitaniya/series_259.html"; }, [{page: 0, timeout: 3000}], prices, 'sources');
    yield run_urls(nightmare, (i) => { return "http://www.deltronics.ru/product/istochniki-pitaniya/series_183.html?accessories=1"; }, [{page: 0, timeout: 3000}], prices, 'sources-accesories');
    yield run_urls(nightmare, (i) => { return "http://www.deltronics.ru/product/paneli-operatora/series_69.html?accessories=1"; }, [{page: 0, timeout: 3000}], prices, 'panels-accesories');
    yield run_urls(nightmare, (i) => { return "http://www.deltronics.ru/product/paneli-operatora/series_217.html?vkl=model-vkl"; }, [{page: 0, timeout: 3000}], prices, 'panels');
    yield run_urls(nightmare, (i) => { return "http://www.deltronics.ru/product/paneli-operatora/series_255.html?vkl=model-vkl"; }, [{page: 0, timeout: 3000}], prices, 'panels');
    yield run_urls(nightmare, (i) => { return "http://www.deltronics.ru/product/paneli-operatora/series_26.html?vkl=model-vkl"; }, [{page: 0, timeout: 3000}], prices, 'panels');
    yield run_urls(nightmare, (i) => { return "http://www.deltronics.ru/product/paneli-operatora/series_173.html?vkl=model-vkl"; }, [{page: 0, timeout: 3000}], prices, 'panels');
    yield run_urls(nightmare, (i) => { return "http://www.deltronics.ru/product/paneli-operatora/series_196.html?vkl=model-vkl"; }, [{page: 0, timeout: 3000}], prices, 'panels');
    yield run_urls(nightmare, (i) => { return "http://www.deltronics.ru/product/scada/scada-series_232.html?vkl=model-vkl"; }, [{page: 0, timeout: 3000}], prices, 'scada');
    yield run_urls(nightmare, (i) => { return "http://www.deltronics.ru/product/fotek/series2_225.html?state_relay=3"; }, [{page: 0, timeout: 3000}], prices, 'radiators');
    yield run_urls(nightmare, (i) => { return "http://www.deltronics.ru/product/fotek/series2_84.html?vkl=model-vkl"; }, [{page: 0, timeout: 3000}], prices, 'relays');
    yield run_urls(nightmare, (i) => { return "http://www.deltronics.ru/product/fotek/series2_84.html?modelPos=30&vkl=model-vkl"; }, [{page: 0, timeout: 3000}], prices, 'relays');
    yield run_urls(nightmare, (i) => { return "http://www.deltronics.ru/product/fotek/series2_83.html?vkl=model-vkl"; }, [{page: 0, timeout: 3000}], prices, 'relays');
    yield run_urls(nightmare, (i) => { return "http://www.deltronics.ru/product/fotek/series2_224.html?state_relay=3"; }, [{page: 0, timeout: 3000}], prices, 'relays');
    yield run_urls(nightmare, (i) => { return "http://www.deltronics.ru/product/fotek/series2_82.html?vkl=model-vkl"; }, [{page: 0, timeout: 3000}], prices, 'relays');
    yield run_urls(nightmare, (i) => { return "http://www.deltronics.ru/product/fotek/series2_82.html?modelPos=30&vkl=model-vkl"; }, [{page: 0, timeout: 3000}], prices, 'relays');
    yield run_urls(nightmare, (i) => { return "http://www.deltronics.ru/product/fotek/series2_82.html?modelPos=60&vkl=model-vkl"; }, [{page: 0, timeout: 1000}], prices, 'relays');
    yield run_urls(nightmare, (i) => { return "http://www.deltronics.ru/product/fotek/series2_117.html?vkl=model-vkl"; }, [{page: 0, timeout: 3000}], prices, 'relays');
    yield run_urls(nightmare, (i) => { return "http://www.deltronics.ru/product/fotek/series2_85.html?vkl=model-vkl"; }, [{page: 0, timeout: 3000}], prices, 'regulators');
    yield run_urls(nightmare, (i) => { return "http://www.deltronics.ru/product/fotek/series2_91.html?vkl=model-vkl"; }, [{page: 0, timeout: 3000}], prices, 'timers');
    yield run_urls(nightmare, (i) => { return "http://www.deltronics.ru/product/temp-controlleri/series_34.html?vkl=model-vkl"; }, [{page: 0, timeout: 3000}], prices, 'termocontrollers');
    yield run_urls(nightmare, (i) => { return "http://www.deltronics.ru/product/controllers/series_240.html?vkl=model-vkl"; }, [{page: 0, timeout: 3000}], prices, 'termocontrollers');
    yield run_urls(nightmare, (i) => { return "http://www.deltronics.ru/product/temp-controlleri/series_31.html?vkl=model-vkl"; }, [{page: 0, timeout: 3000}], prices, 'termocontrollers');
    yield run_urls(nightmare, (i) => { return "http://www.deltronics.ru/product/temp-controlleri/series_184.html?vkl=model-vkl"; }, [{page: 0, timeout: 3000}], prices, 'termocontrollers');
    yield run_urls(nightmare, (i) => { return "http://www.deltronics.ru/product/temp-controlleri/series_166.html?vkl=model-vkl"; }, [{page: 0, timeout: 3000}], prices, 'termocontrollers');
    yield run_urls(nightmare, (i) => { return "http://www.deltronics.ru/product/fotek/series2_95.html?vkl=model-vkl"; }, [{page: 0, timeout: 3000}], prices, 'sensors');
    yield run_urls(nightmare, (i) => { return "http://www.deltronics.ru/product/fotek/series2_99.html?vkl=model-vkl"; }, [{page: 0, timeout: 3000}], prices, 'sensors');
    yield run_urls(nightmare, (i) => { return "http://www.deltronics.ru/product/fotek/series2_97.html?vkl=model-vkl"; }, [{page: 0, timeout: 3000}], prices, 'sensors');
    yield run_urls(nightmare, (i) => { return "http://www.deltronics.ru/product/fotek/series2_96.html?vkl=model-vkl"; }, [{page: 0, timeout: 3000}], prices, 'sensors');
    yield run_urls(nightmare, (i) => { return `http://www.deltronics.ru/product/fotek/series2_94.html?modelPos=${i}&vkl=model-vkl`}, [
      {page: 0, timeout: 3000}, {page: 1, timeout: 1000}, {page: 2, timeout: 1000}, {page: 3, timeout: 1000}, {page: 4, timeout: 1000},
      {page: 5, timeout: 1000}, {page: 6, timeout: 1000}, {page: 7, timeout: 1000}
    ], prices, 'sensors');
    yield run_urls(nightmare, (i) => { return "http://www.deltronics.ru/product/fotek/series2_93.html?vkl=model-vkl"; }, [{page: 0, timeout: 3000}], prices, 'sensors');
    yield run_urls(nightmare, (i) => { return "http://www.deltronics.ru/product/sensors/series_203.html?vkl=model-vkl"; }, [{page: 0, timeout: 3000}], prices, 'sensors');
    yield run_urls(nightmare, (i) => { return "http://www.deltronics.ru/product/sensors/series_200.html?vkl=model-vkl"; }, [{page: 0, timeout: 3000}], prices, 'inductive-sensors');
    yield run_urls(nightmare, (i) => { return "http://www.deltronics.ru/product/sensors/series_199.html?vkl=model-vkl"; }, [{page: 0, timeout: 3000}], prices, 'sensors');
    yield run_urls(nightmare, (i) => { return "http://www.deltronics.ru/product/sensors/series_213.html?vkl=model-vkl"; }, [{page: 0, timeout: 3000}], prices, 'sensors');
    yield run_urls(nightmare, (i) => { return "http://www.deltronics.ru/product/sensors/series_214.html?vkl=model-vkl"; }, [{page: 0, timeout: 3000}], prices, 'sensors');
    yield run_urls(nightmare, (i) => { return `http://www.deltronics.ru/product/controllers/series_210.html?modelPos=${i}&vkl=model-vkl`}, [
      {page: 0, timeout: 3000}, {page: 1, timeout: 1000}, 
    ], prices, 'controllers-as3000');
    yield run_urls(nightmare, (i) => { return "http://www.deltronics.ru/product/controllers/series_42.html?vkl=model-vkl"; }, [{page: 0, timeout: 3000}], prices, 'controllers-dvp-ec3');
    yield run_urls(nightmare, (i) => { return "http://www.deltronics.ru/product/controllers/series_74.html?vkl=model-vkl&accessories=1"; }, [{page: 0, timeout: 3000}], prices, 'dvp-accesories');
    yield run_urls(nightmare, (i) => { return "http://www.deltronics.ru/product/controllers/series_123.html?accessories=1"; }, [{page: 0, timeout: 3000}], prices, 'dvp-accesories');
    yield run_urls(nightmare, (i) => { return "http://www.deltronics.ru/product/controllers/series_41.html?vkl=model-vkl"; }, [{page: 0, timeout: 3000}], prices, 'controllers-dvp-es2');
    yield run_urls(nightmare, (i) => { return "http://www.deltronics.ru/product/controllers/series_124.html?vkl=model-vkl&accessories=1"; }, [{page: 0, timeout: 3000}], prices, 'dvp-remote-accesories');
    yield run_urls(nightmare, (i) => { return `http://www.deltronics.ru/product/controllers/series_113.html?modelPos=${i}&vkl=model-vkl`}, [
      {page: 0, timeout: 3000}, {page: 1, timeout: 1000}, 
    ], prices, 'controllers-ah500');
    yield run_urls(nightmare, (i) => { return `http://www.deltronics.ru/product/converter/series_63.html?modelPos=${i}&vkl=model-vkl&accessories=1`}, [
      {page: 0, timeout: 3000}, {page: 1, timeout: 1000}, {page: 2, timeout: 1000}
    ], prices, 'controllers-ah500');
    yield run_urls(nightmare, (i) => { return "http://www.deltronics.ru/product/controllers/series_258.html"; }, [{page: 0, timeout: 3000}], prices, 'controllers-ah500');
    yield run_urls(nightmare, (i) => { return `http://www.deltronics.ru/product/converter/series_9.html?modelPos=${i}&vkl=model-vkl`}, [
      {page: 0, timeout: 3000}, {page: 1, timeout: 1000}, {page: 2, timeout: 1000} 
    ], prices, 'invertors-c2000');
    yield run_urls(nightmare, (i) => { return "http://www.deltronics.ru/product/converter/series_65.html?vkl=model-vkl&accessories=1"; }, [{page: 0, timeout: 3000}], prices, 'invertors-accesories');
    yield run_urls(nightmare, (i) => { return `http://www.deltronics.ru/product/converter/series_66.html?modelPos=${i}&vkl=model-vkl&accessories=1`}, [
      {page: 0, timeout: 3000}, {page: 1, timeout: 1000}, 
    ], prices, 'inverters-accesories');
    yield run_urls(nightmare, (i) => { return `http://www.deltronics.ru/product/converter/series_62.html?modelPos=${i}&vkl=model-vkl&accessories=1`}, [
      {page: 0, timeout: 3000}, {page: 1, timeout: 1000}, 
    ], prices, 'inverters-drossels');
    yield run_urls(nightmare, (i) => { return `http://www.deltronics.ru/product/converter/series_64.html?modelPos=${i}&vkl=model-vkl&accessories=1`}, [
      {page: 0, timeout: 3000}, {page: 1, timeout: 1000}, 
    ], prices, 'inverters-filters');
    yield run_urls(nightmare, (i) => { return `http://www.deltronics.ru/product/converter/series_63.html?modelPos=${i}&vkl=model-vkl&accessories=1`}, [
      {page: 0, timeout: 3000}, {page: 1, timeout: 1000}, {page: 2, timeout: 1000}
    ], prices, 'inverters-resistors');
    yield run_urls(nightmare, (i) => { return "http://www.deltronics.ru/product/converter/series_67.html?vkl=model-vkl&accessories=1"; }, [{page: 0, timeout: 2000}], prices, 'invertors-accesories');
    yield run_urls(nightmare, (i) => { return "http://www.deltronics.ru/product/converter/series_219.html?vkl=model-vkl"; }, [{page: 0, timeout: 2000}], prices, 'invertors-vkl');


    let jsonContent = JSON.stringify(prices);
    fs.writeFileSync("asd-a2.json", jsonContent, 'utf8', function (err) {
        if (err) {
            console.log("An error occured while writing JSON Object to File.");
            return console.log(err);
        }
        console.log("JSON file has been saved.");
    });

    yield nightmare.end();
}
