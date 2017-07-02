package mooltipass.automatedTest.pageObjects;

import org.openqa.selenium.By;
import org.openqa.selenium.JavascriptExecutor;
import org.openqa.selenium.NoSuchElementException;
import org.openqa.selenium.WebDriver;
import org.openqa.selenium.WebElement;
import org.openqa.selenium.interactions.Actions;
import org.openqa.selenium.support.ui.ExpectedCondition;
import org.openqa.selenium.support.ui.ExpectedConditions;
import org.openqa.selenium.support.ui.WebDriverWait;


public class AbstractPage {
	protected WebDriver driver;
	protected WebDriverWait wait;

	public AbstractPage(WebDriver driver) {
		this.driver = driver;
		wait = new WebDriverWait(driver, 30);
	}

	public static void sleep(int ms) {
		try {
			Thread.sleep(ms);
		} catch (InterruptedException e) {
			// TODO Auto-generated catch block
			e.printStackTrace();
		}
	}

	protected boolean isElementPresent(By by) {
		try {
			driver.findElement(by);
			return true;
		} catch (NoSuchElementException e) {
			return false;
		}
	}

	protected void enter(WebElement element, String keys) {
		element.clear();
		element.sendKeys(keys);	
	}
	
	
	
	protected void click(WebElement element) {
		element.click();
	}
	
	protected void waitUntilAppears(By by){
		try{
			WebDriverWait wait = new WebDriverWait(driver, 12);
			wait.until(ExpectedConditions.elementToBeClickable(by));
		}catch(Exception e)
		{
			
		}
	}
	protected boolean waitUntilAppears(WebElement element){
		try{
			WebDriverWait wait = new WebDriverWait(driver, 12);
			wait.until(ExpectedConditions.elementToBeClickable(element));
			return true;
		}
		catch(Exception e)
		{
			return false;
		}
     }
	
	protected void clickWithAction(WebElement element)
	{
		Actions actions = new Actions(driver);
		actions.moveToElement(element).click().perform();
	}
	
	protected void hover(WebElement element)
	{
		
		Actions actions = new Actions(driver);
		actions.moveToElement(element).build().perform();
	}
	
	protected void clickWithJS(WebElement element){

		JavascriptExecutor executor = (JavascriptExecutor) driver;
		executor.executeScript("arguments[0].click();", element);
	}
}


