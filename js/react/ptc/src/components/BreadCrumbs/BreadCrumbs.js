import React from 'react';
import { Breadcrumb } from 'antd';
import { Link } from 'react-router-dom';
import breadcrumbs from '../../img/outdoor_furniture/bx-breadcrumbs.svg';
import '../../containers/Base/Partners/Style/style.css';
import styled from 'styled-components';

function BreadCrumbs({ links, fromRoot }) {
  return (
    <StyledBreadcrumb>
      <StyledBreadcrumbItem>
        <img alt="" src={breadcrumbs} style={{ margin: '0 8px 0 0' }} />
        <StyledLink to="/">Главная</StyledLink>
      </StyledBreadcrumbItem>
      {links.map((link, i) => {
        return (
          <StyledBreadcrumbItem key={i}>
            <StyledLink to={`${fromRoot ? '' : '/'}${link.id}`}>{link.value}</StyledLink>
          </StyledBreadcrumbItem>
        );
      })}
    </StyledBreadcrumb>
  );
}

export function BreadCrumbsRoutes({ links, keys = undefined }) {
  return (
    <StyledBreadcrumb>
      {/*<StyledBreadcrumbItem>*/}
      {/*<img alt='' src={breadcrumbs} style={{ margin: '0 8px 0 0' }} />*/}
      {/*<StyledLink to="/">Главная</StyledLink>*/}
      {/*</StyledBreadcrumbItem>*/}
      {links.map((link, i) => {
        let k = link.name || link.title;
        let spec = {
          title: link.name || link.title || '',
          path: link.path,
          nopath: false,
        };
        if (keys && k && k in keys) {
          let titleArg = keys[k].titleArg;
          if (titleArg) {
            spec.title = link.generateName(titleArg);
          }
          let urlArg = keys[k].urlArg;
          if (urlArg) {
            spec.path = link.url(urlArg);
          }
          spec.nopath = keys[k].nopath ? true : false;
        }
        return (
          <StyledBreadcrumbItem key={i}>
            {i === 0 && <img alt="" src={breadcrumbs} style={{ margin: '0 8px 0 0' }} />}
            {spec.nopath ? (
              <StyledSpanLink>{spec.title}</StyledSpanLink>
            ) : (
              <StyledLink to={spec.path}>{spec.title}</StyledLink>
            )}
          </StyledBreadcrumbItem>
        );
      })}
    </StyledBreadcrumb>
  );
}

const StyledLink = styled(Link)`
  color: #8aa1c1 !important;
`;
const StyledSpanLink = styled.span`
  color: #8aa1c1 !important;
`;
const StyledBreadcrumbItem = styled(Breadcrumb.Item)`
  color: #8aa1c1 !important;
`;
const StyledBreadcrumb = styled(Breadcrumb)`
  font-size: 11px;
  margin: 20px 0 20px 0;
`;

export default BreadCrumbs;
