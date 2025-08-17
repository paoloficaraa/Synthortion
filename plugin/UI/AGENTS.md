# Agent Development Guide

This UI project includes AI agents to help with development and deployment.

## Available Agents

### Organization Agent

- **Purpose**: Organize and clean up the UI codebase
- **Usage**: The agent helps maintain clean code structure and removes unused dependencies
- **Configuration**: See `.builder/rules/organize-ui.mdc` for specific rules

### Deployment Agent

- **Purpose**: Handle application deployment to various platforms
- **Usage**: Automates deployment processes and environment setup
- **Configuration**: See `.builder/rules/deploy-app.mdc` for deployment rules

## How to Work with Agents

1. The agents follow rules defined in the `.builder/rules/` directory
2. Each agent has specific responsibilities and capabilities
3. Agents can be invoked through VS Code extensions or command line tools
4. Always review agent suggestions before applying changes

## Best Practices

- Keep agent rules updated as the project evolves
- Test agent-generated code before committing
- Use agents for repetitive tasks and code organization
- Maintain clear separation between agent-managed and manually-managed code
