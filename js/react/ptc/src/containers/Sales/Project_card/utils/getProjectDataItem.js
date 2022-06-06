export const getProjectDataItem = (dataProjectCard) =>
  dataProjectCard?.searchProject?.edges && dataProjectCard?.searchProject.edges.length > 0
    ? dataProjectCard?.searchProject?.edges[0].node
    : [];
