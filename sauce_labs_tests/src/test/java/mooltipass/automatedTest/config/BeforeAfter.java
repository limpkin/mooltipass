package mooltipass.automatedTest.config;

import java.io.File;
import java.io.IOException;
import java.util.concurrent.TimeUnit;

import org.openqa.selenium.OutputType;
import org.openqa.selenium.TakesScreenshot;
import org.openqa.selenium.WebDriver;
import org.openqa.selenium.remote.RemoteWebDriver;
import cucumber.api.Scenario;
import cucumber.api.java.After;
import cucumber.api.java.Before;


public class BeforeAfter {

	protected static WebDriver driver;

	@Before
	public void beforeScenario(Scenario scenario) {
		getNewBrowser(scenario.getName());

	}

	@After
	public void afterScenario(Scenario scenario) {
//		if (scenario.isFailed()) {
//			try {
//				takeScreenshot(scenario.getName(), scenario.getId());
//			} catch (IOException e) {
//				e.printStackTrace();
//			}
//		}
		System.out.println(scenario.getName()+" RUNNING AT: https://saucelabs.com/beta/tests/"+((RemoteWebDriver) driver).getSessionId().toString());	
		closeBrowser();
	}



	/**
	 * Opens a new browser window and returns a new driver.
	 */
	protected static void getNewBrowser(String name) {
		driver = WebDriverFactory.get(name);
		driver.manage().timeouts().implicitlyWait(5, TimeUnit.SECONDS);
	}

	/**
	 * Closes the current browser (driver).
	 */
	protected static void closeBrowser() {
		driver.quit();
		driver = null;
	}




	protected void takeScreenshot(String scenarioName, String scenarioId)
			throws IOException {
		driver = WebDriverFactory.get();
		File scrFile = ( (TakesScreenshot) driver).getScreenshotAs(OutputType.FILE);
		
		scrFile.renameTo(new File("./screenshots/",
				Long.toHexString(System.currentTimeMillis())+scenarioName + "[" + scenarioId + "]_screenshot.png"));
		
	}
}