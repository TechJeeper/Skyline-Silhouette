module.exports = [
  {
    type: 'heading',
    defaultValue: 'Skyline Silhouette'
  },
  {
    type: 'text',
    defaultValue:
      'A new city every morning. Building sprites shuffle into a fresh skyline at midnight.'
  },

  {
    type: 'section',
    items: [
      {
        type: 'heading',
        defaultValue: 'Time'
      },
      {
        type: 'select',
        messageKey: 'TimeFormat',
        defaultValue: '0',
        label: 'Hour format',
        options: [
          { label: 'System default', value: '0' },
          { label: '12-hour', value: '1' },
          { label: '24-hour', value: '2' }
        ]
      },
      {
        type: 'toggle',
        messageKey: 'LeadingZero',
        label: 'Leading zero on hours',
        defaultValue: true
      },
      {
        type: 'toggle',
        messageKey: 'ShowDate',
        label: 'Show date',
        defaultValue: true
      }
    ]
  },

  {
    type: 'section',
    items: [
      {
        type: 'heading',
        defaultValue: 'Skyline'
      },
      {
        type: 'select',
        messageKey: 'Density',
        defaultValue: '1',
        label: 'Building density',
        options: [
          { label: 'Sparse', value: '0' },
          { label: 'Normal', value: '1' },
          { label: 'Dense', value: '2' }
        ]
      },
      {
        type: 'slider',
        messageKey: 'CitySeed',
        label: 'City variation',
        description: 'Nudge the seed for a different skyline today (still refreshes at midnight).',
        defaultValue: 0,
        min: 0,
        max: 99,
        step: 1
      }
    ]
  },

  {
    type: 'section',
    items: [
      {
        type: 'heading',
        defaultValue: 'Sky'
      },
      {
        type: 'select',
        messageKey: 'SkyMode',
        defaultValue: '0',
        label: 'Time of day',
        options: [
          { label: 'Auto (by hour)', value: '0' },
          { label: 'Dawn', value: '1' },
          { label: 'Day', value: '2' },
          { label: 'Dusk', value: '3' },
          { label: 'Night', value: '4' }
        ]
      },
      {
        type: 'toggle',
        messageKey: 'ShowCelestial',
        label: 'Show sun / moon',
        defaultValue: true
      },
      {
        type: 'toggle',
        messageKey: 'ShowStars',
        label: 'Show stars at night',
        defaultValue: true
      },
      {
        type: 'toggle',
        messageKey: 'InvertBw',
        label: 'Invert on black & white',
        description: 'For Pebble 2 / Time 2 Duo style displays.',
        defaultValue: false
      }
    ]
  },

  {
    type: 'submit',
    defaultValue: 'Save'
  }
];
