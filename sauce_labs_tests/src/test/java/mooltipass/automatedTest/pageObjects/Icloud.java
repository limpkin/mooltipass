package mooltipass.automatedTest.pageObjects;

import org.openqa.selenium.By;
import org.openqa.selenium.WebDriver;
import org.openqa.selenium.WebElement;
import org.openqa.selenium.interactions.Action;
import org.openqa.selenium.interactions.Actions;
import org.openqa.selenium.support.FindBy;
import org.openqa.selenium.support.PageFactory;
public class Icloud extends AbstractPage{
	
	public Icloud (WebDriver driver) {
		super(driver);
		PageFactory.initElements(driver, this);
	}
	
	@FindBy(xpath = "//input[@id='appleId']")
	private WebElement email;

	@FindBy(xpath = "//input[@id='pwd']")
	private WebElement password;
	
	@FindBy(xpath = "//button[@id='sign-in']//i[@class='icon icon_sign_in']")
	private WebElement submitLogin;
	
	@FindBy(xpath = "//div[@title='iCloud Settings & Sign Out']")
	private WebElement menuDiv;
	
	@FindBy(xpath = "//span[text()='Sign Out']")
	private WebElement logoutDiv;
	
	@FindBy(xpath = "//a[@aria-label='Close']")
	private WebElement popUp;
	
			
	public void enterEmail(String value){
		driver.switchTo().frame(driver.findElement(By.id("auth-frame")));
		waitUntilAppears(email);
		email.sendKeys(value);
	}

	public void enterPassword(String value){

		waitUntilAppears(password);
		password.sendKeys(value);
	}
	
	public void submit(){
		waitUntilAppears(submitLogin);
		clickWithJS(submitLogin);
	}
	
	public void logout(){
		waitUntilAppears(menuDiv);
		menuDiv.click();
		waitUntilAppears(logoutDiv);
		logoutDiv.click();
	}
	
	public boolean checkAtLoginPage(){
		driver.switchTo().frame(driver.findElement(By.id("auth-frame")));
		waitUntilAppears(email);
		return isElementPresent(By.xpath("//input[@id='appleId']"));
	}

	
	public boolean checkLogin(){
		driver.switchTo().defaultContent();
		waitUntilAppears(By.xpath("//div[@title='iCloud Settings & Sign Out']"));
		return isElementPresent(By.xpath("//div[@title='iCloud Settings & Sign Out']"));
	}
}