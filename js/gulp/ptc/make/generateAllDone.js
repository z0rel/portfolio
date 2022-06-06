module.exports = function generateAllDone(cb) {
  return (done) => {
    done();
    cb();
  };
};
