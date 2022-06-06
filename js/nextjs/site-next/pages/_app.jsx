import "../scss/main_global.scss";
import "swiper/css";
import "swiper/css/pagination";
import "swiper/css/navigation";

import {useEffect, useState} from "react";
import {detectWebpSupport} from "../components/utils/detectWebpSupport";
import {GlobalContext} from "../components/utils/GlobalContext";



function MyApp({ Component, pageProps }) {
  let [webpackUsingResolved, setWebpackUsingResolved] = useState({state: null, resolved: false})

  useEffect(() => {
    detectWebpSupport().then((d) => {
        setWebpackUsingResolved({state: d, resolved: true})
    });
  })


  return !webpackUsingResolved.resolved
      ? null
      : <GlobalContext.Provider value={{webpack: webpackUsingResolved.state}}>
          <Component {...pageProps} />
      </GlobalContext.Provider>;
}

export default MyApp;
