import { useEffect, useState } from 'react';
import Button from 'react-bootstrap/Button';
import Form from 'react-bootstrap/Form';
import { useTranslation } from 'react-i18next';

import Section from '../Components/Section';
import WebApi from '../Services/WebApi';

const LED_BUTTONS = [
	'Left',
	'Down',
	'Right',
	'Up',
	'B3',
	'B4',
	'R1',
	'L1',
	'B1',
	'B2',
	'R2',
	'L2',
	'R3',
	'L3',
];

const KEY_FX = [0, 1, 2, 3, 4, 5];
const PRESETS = Array.from({ length: 30 }, (_, i) => i);

const emptyKey = () => ({ normal: '#ff1010', pressed: '#ffffff', effect: 1 });

export default function RgbMenuPage() {
	const { t } = useTranslation('');
	const [preset, setPreset] = useState(0);
	const [useCustom, setUseCustom] = useState(false);
	const [theme, setTheme] = useState({});
	const [saveMessage, setSaveMessage] = useState('');
	const [saving, setSaving] = useState(false);
	const [loaded, setLoaded] = useState(false);

	useEffect(() => {
		WebApi.getCustomTheme(() => {}).then((data) => {
			if (!data) return;
			const index = Number(data.baseAnimationIndex) || 0;
			setUseCustom(index === 30);
			setPreset(index >= 30 ? 0 : index);
			const next = { ...(data.customTheme || {}) };
			LED_BUTTONS.forEach((key) => {
				next[key] = {
					normal: next[key]?.normal || '#ff1010',
					pressed: next[key]?.pressed || '#ffffff',
					effect: Math.min(5, Number(next[key]?.effect ?? 1) || 0),
				};
			});
			setTheme(next);
			setLoaded(true);
		});
	}, []);

	const updateKey = (key, patch) => {
		setTheme((current) => ({
			...current,
			[key]: { ...(current[key] || emptyKey()), ...patch },
		}));
	};

	const save = async () => {
		setSaving(true);
		setSaveMessage('');
		const ok = await WebApi.setCustomTheme({
			hasCustomTheme: true,
			baseAnimationIndex: useCustom ? 30 : preset,
			customTheme: theme,
		});
		setSaving(false);
		setSaveMessage(ok ? t('RgbMenu:saved-text') : t('RgbMenu:failed-text'));
	};

	return (
		<Section title={t('RgbMenu:header-text')}>
			<Form.Group className="mb-3">
				<Form.Label>{t('RgbMenu:preset-label')}</Form.Label>
				<Form.Select
					value={preset}
					disabled={useCustom}
					onChange={(e) => setPreset(Number(e.target.value))}
				>
					{PRESETS.map((id) => (
						<option key={id} value={id}>
							{t(`RgbMenu:effect-${id}`)}
						</option>
					))}
				</Form.Select>
				<Form.Text className="d-block mt-2">{t('RgbMenu:preset-help')}</Form.Text>
			</Form.Group>

			<h3 className="h5 mt-4">{t('RgbMenu:custom-title')}</h3>
			<p className="text-body-secondary">{t('RgbMenu:custom-help')}</p>
			<Form.Check
				className="mb-3"
				type="switch"
				id="use-custom-lights"
				label={t('RgbMenu:use-custom')}
				checked={useCustom}
				onChange={(e) => setUseCustom(e.target.checked)}
			/>

			<div className="table-responsive">
				<table className="table table-sm align-middle">
					<thead>
						<tr>
							<th>{t('RgbMenu:col-button')}</th>
							<th>{t('RgbMenu:col-color')}</th>
							<th>{t('RgbMenu:col-effect')}</th>
							<th>{t('RgbMenu:col-press')}</th>
						</tr>
					</thead>
					<tbody>
						{LED_BUTTONS.map((key) => {
							const row = theme[key] || emptyKey();
							return (
								<tr key={key}>
									<td>{t(`RgbMenu:btn-${key}`)}</td>
									<td>
										<input
											type="color"
											className="form-control form-control-color"
											value={row.normal}
											aria-label={t('RgbMenu:col-color')}
											onChange={(e) => updateKey(key, { normal: e.target.value })}
										/>
									</td>
									<td>
										<Form.Select
											value={row.effect}
											aria-label={t('RgbMenu:col-effect')}
											onChange={(e) =>
												updateKey(key, { effect: Number(e.target.value) })
											}
										>
											{KEY_FX.map((id) => (
												<option key={id} value={id}>
													{t(`RgbMenu:fx-${id}`)}
												</option>
											))}
										</Form.Select>
									</td>
									<td>
										<input
											type="color"
											className="form-control form-control-color"
											value={row.pressed}
											aria-label={t('RgbMenu:col-press')}
											onChange={(e) => updateKey(key, { pressed: e.target.value })}
										/>
									</td>
								</tr>
							);
						})}
					</tbody>
				</table>
			</div>

			<Button variant="primary" onClick={save} disabled={saving || !loaded}>
				{t('RgbMenu:save-button')}
			</Button>
			{saveMessage ? <span className="ms-3">{saveMessage}</span> : null}
		</Section>
	);
}
