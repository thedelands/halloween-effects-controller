# Pull Request Template

## Summary

<!-- What does this pull request change, and why? -->

## Type of change

- [ ] Bug fix
- [ ] New feature
- [ ] Firmware change
- [ ] Hardware or wiring change
- [ ] Documentation update
- [ ] Build, tooling, or maintenance change

## Related issue

<!-- Example: Closes #123 -->

## Testing

<!-- Describe how the change was tested. Include hardware used when relevant. -->

- [ ] Firmware builds successfully
- [ ] Tested on physical hardware
- [ ] Web interface tested
- [ ] Existing behavior remains functional
- [ ] Documentation updated

### Test environment

- Board:
- PlatformIO environment:
- Relevant peripherals:
- Browser/device:
- Other details:

## Hardware and safety impact

- [ ] No hardware or safety impact
- [ ] Wiring or pin assignments changed
- [ ] Power requirements changed
- [ ] New electrical or mechanical risks were considered
- [ ] Relevant diagrams and safety documentation were updated

<!-- Explain any hardware or safety implications. -->

## Screenshots, logs, or diagrams

<!-- Add supporting evidence when useful. Remove sensitive configuration data. -->

## Contributor checklist

- [ ] My changes are focused and clearly described
- [ ] I reviewed `docs/01-safety-and-design-decisions.md`
- [ ] I did not commit secrets or `firmware/include/fog_controller_config.h`
- [ ] I updated `CHANGELOG.md` when appropriate
- [ ] I documented anything I could not test
