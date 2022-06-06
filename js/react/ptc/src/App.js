import React from 'react';
import { ConfigProvider } from 'antd';
import { BrowserRouter } from 'react-router-dom';
import { Route, Switch } from 'react-router';
import locale from 'antd/es/locale/ru_RU';
import './App.css';
import 'antd/dist/antd.css';
import Header from './components/Header/Header';

import { routes } from './routes';

const App = () => {
  let routesArr = [];
  for (let [keyTop, routeTop] of Object.entries(routes)) {
    for (let [keyValue, routeValue] of Object.entries(routeTop)) {
      routesArr.push(
        <Route key={keyTop + keyValue} path={routeValue.path} exact component={routeValue.component} />
      )
    }
  }
  return (
    <ConfigProvider locale={locale}>
      <BrowserRouter>
        <Header />
        <Switch>{routesArr}</Switch>
      </BrowserRouter>
    </ConfigProvider>
  );
};

export default App;
