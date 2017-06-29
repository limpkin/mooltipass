package mooltipass.automatedTest.pageObjects;

import org.openqa.selenium.By;
import org.openqa.selenium.WebDriver;
import org.openqa.selenium.WebElement;
import org.openqa.selenium.support.FindBy;
import org.openqa.selenium.support.PageFactory;

public class Tripadvisor extends AbstractPage{

	public Tripadvisor(WebDriver driver) {
		super(driver);
		PageFactory.initElements(driver, this);
	}

	@FindBy(className = "login-button")
	private WebElement loginBtn;

	@FindBy(xpath = "//input[@id='regSignIn.email']")
	private WebElement email;

	@FindBy(xpath = "//input[@id='regSignIn.password']")
	private WebElement password;

	@FindBy(xpath = "//div[text()='Log In']")
	private WebElement submitLogin;
	
	@FindBy(xpath = "//a[@data-tracking-label='UserProfile_signout']")
	private WebElement logoutBtn;
	
	@FindBy(xpath = "//div[@title='Profile']")
	private WebElement user;
	
	public void goToLogin(){
		loginBtn.click();
	}
	
	public void enterEmail(String value){
		driver.switchTo().frame(driver.findElement(By.id("overlayRegFrame")));
		email.sendKeys(value);
	}

	public void enterPassword(String value){
		password.sendKeys(value);
	}
	
	public void submit(){
		submitLogin.click();
		driver.switchTo().defaultContent();
		}
	
	public boolean checkLogin(){
		
		waitUntilAppears(By.xpath("//span[contains(text(),'citesting')]"));
		return isElementPresent(By.xpath("//span[contains(text(),'citesting')]"));
	}


	public boolean checkAtLoginPage(){
		driver.switchTo().frame(driver.findElement(By.id("overlayRegFrame")));
		waitUntilAppears(By.xpath("//input[@id='regSignIn.email']"));
		return isElementPresent(By.xpath("//input[@id='regSignIn.email']"));
	}
	
	public void logout(){
		user.click();
		logoutBtn.click();
		
	}
}
